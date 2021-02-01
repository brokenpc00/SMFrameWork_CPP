package com.brokenpc.SMCocosLib;

/**
 * Created by stevemac on 2017. 10. 17..
 */

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Point;
import android.graphics.PointF;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.AutoFocusMoveCallback;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.ShutterCallback;
import android.hardware.Camera.Size;
import android.util.Log;
import android.util.SparseArray;

import com.brokenpc.SMCocosLib.CameraHelper.CameraCaps;
import com.brokenpc.SMCocosLib.CameraHelper.CameraInfo2;

import org.cocos2dx.cpp.AppActivity;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

// for bar code
import com.google.android.gms.vision.Frame;
import com.google.android.gms.vision.barcode.Barcode;
import com.google.android.gms.vision.barcode.BarcodeDetector;


/**
 * Created by KimJaePhil_iMac on 2017. 7. 24..
 * 여기서 자동으로 뭐 하지마!!! 외부에서 다 호출 할 거야
 * 단순한 것만 interface 빼준다
 */

@SuppressWarnings("deprecation")
public class DeviceCameraManager implements PreviewCallback,
        PictureCallback,
        ShutterCallback
{
    public interface CameraPreviewListener {
        void onStartCamera(int cameraId);   // 카메라가 시작됐다
        void onStopCamera();   // 카메라가 종료됐다
        void onPauseCamera(); // 카메라가 멈췄다.
        void onResumeCamera(); // 카메라가 재개됐다.
        void onSwitchCameraStarted(); // 카메라 전환 시작됐다.
        void onSwitchCameraFinished(); // 카메라 전환 완료됐다.
        void onPreviewFrame(byte[] data); // preview frame 전달
        void onPictureTaken(byte[] data); // 사진촬영
        void onShutter();   // shutter가 눌렸다. 여기서는 아무것도 안하는걸로...
    }

    public void setOnCameraPreviewListener(CameraPreviewListener l) {
        mListener = l;
    }
    private CameraPreviewListener mListener;

    private boolean mFullScreenMode = false;
    private float mPreviewPosX;
    private float mPreviewPosY;

    private static final int INITIAL_CAMERA_ID = 0;

    private volatile Camera mCameraInstance;
    private CameraHelper mCameraHelper;
//    private CameraOpenThread mCameraOpenThread = null;

    private int mCurrentCameraId = INITIAL_CAMERA_ID;
    private int mDrawCameraId = INITIAL_CAMERA_ID;

    // preview sprite 대신에 buffer 2개 필요
    CameraPreviewBuffer[] mPreviewBuffer;

    private int mOpenCameraId = -1;
    private int mOpenWidth;
    private int mOpenHeight;
    private int mOpenOrientation;
    private boolean mOpenFrontFacing;

    private int mNotifyFirstFrameState = 0;  // 0 : 아직 안받음, 1 : onPreviewFrame에서 받음, 2 : updateFrame 했음..-> 여기서는 다른걸로 대체해야함

    private boolean mPreviewRunning = false;


    private AutoFocusMoveCallback mAutofocusMoveCallback;
    private AutoFocusCallback mAutofocusCallback;
    private int mWidth;
    private int mHeight;

    private static AppActivity mActivity = null;


    // for barcode detected
    public BarcodeDetector detector = null;
    public boolean isScannerSupport = false;

    public void setScanSupport(boolean bSet) {
        isScannerSupport = bSet;
    }

    public DeviceCameraManager(Context context, int width, int height, AutoFocusCallback autofocusCallback) {
        mActivity = (AppActivity)context;
        mAutofocusCallback = autofocusCallback;
        // fullscreen Mode는 CameraOpenThread에서 화면 크기 계산할 때 사용 front이거나 fullscreen이 아니면 4:3 비율로 preview를 한다.
        mFullScreenMode = true;

        mWidth = 640;
        mHeight = 480;

        mPreviewPosX = mWidth/2f;
        mPreviewPosY = mHeight/2f;
        isScannerSupport = false;
//.setBarcodeFormats(Barcode.DATA_MATRIX | Barcode.AZTEC | Barcode.PDF417 | Barcode.QR_CODE | Barcode.PRODUCT | Barcode.ISBN | Barcode.ITF )
        detector = new BarcodeDetector.Builder(context)
                .setBarcodeFormats(Barcode.ALL_FORMATS)
                .build();

        if (!detector.isOperational()) {
            Log.i("TEST", "[[[[[ failed to setup barcode detector!!!");
            return;
        }

//        Log.i("CAMERA MANGER", "[[[[[[[[[[[[[[ open width : " + Integer.toString(mWidth) + ", open height : " + Integer.toString(mHeight));
    }

    public DeviceCameraManager(Context context, int width, int height, AutoFocusCallback autoFocusCallback, AutoFocusMoveCallback autoFocusMoveCallback) {
        this (context, width, height, autoFocusCallback);
        mAutofocusMoveCallback = autoFocusMoveCallback;
    }

    // resume 시에도 initCamera를 사용하는데 이거는 제일 처음 한번 호출 될 때만
    public void initCameraOnce(int cameraId) {
        mCameraHelper = new CameraHelper(mActivity);
        int numOfCameras = mCameraHelper.getNumberOfCameras();
        if (cameraId < 0 || cameraId >= numOfCameras) {
            cameraId = 0;
        }
        mCurrentCameraId = cameraId;
        mDrawCameraId = cameraId;
        mPreviewBuffer = new CameraPreviewBuffer[numOfCameras];
        for (int i=0; i<numOfCameras; i++) {
            mPreviewBuffer[i] = null;
        }



        initCamera();
    }

    public int getCameraId() {
        return mCurrentCameraId;
    }

    public void releaseResources() {

        if (mPreviewBuffer != null) {
            for (int i=0; i<mPreviewBuffer.length; i++) {
                if (mPreviewBuffer[i] != null) {
                    mPreviewBuffer[i].releaseBuffer();
                    mPreviewBuffer[i].removeBuffer();
                    mPreviewBuffer[i] = null;
                }
            }

            mPreviewBuffer = null;
        }

        releaseCamera();
    }

    private void initCamera() {

        Log.i("CAMERA MANAGER", "Init Camera 1");
//        if (mCameraInstance==null) {
//            Log.i("CAMERA MANAGER", "Init Camera 1-1.... camera instance is nullllllllll");
//        }
//        try {
//            if (mPreviewBuffer[mCurrentCameraId] == null) {
//                mPreviewBuffer[mCurrentCameraId] = CameraPreviewBuffer.createPreviewBuffer(mOpenHeight, mOpenWidth, mOpenWidth, mOpenHeight, mOpenOrientation, mOpenFrontFacing?true:false, false);
//            }
//
//            for (int i=0; i<CameraPreviewBuffer.NUM_PREVIEW_BUFFER; i++) {
//                Log.i("CAMERA MANAGER", "[[[[[ Init Camera 2 : " + Integer.toString(i) + ", camera id : " + Integer.toString(mCurrentCameraId));
//                if (mPreviewBuffer[mCurrentCameraId]==null) {
//                    Log.i("CAMERA MANAGER", "[[[[[ buffer is null : " + Integer.toString(mCurrentCameraId) + ", index : " + Integer.toString(i));
//                }
//                mCameraInstance.addCallbackBuffer(mPreviewBuffer[mCurrentCameraId].getDataBuffer(i));
//            }
//            Log.i("CAMERA MANAGER", "[[[[[ Init Camera 3");
//            mCameraInstance.setPreviewCallbackWithBuffer(this);
//            Log.i("CAMERA MANAGER", "[[[[[ Init Camera 4");
//        } catch (Exception e) {
//            Log.i("CAMERA MANAGER", "[[[[[ Init Camera exceptions!!!!!!");
//            return;
//        }
    }

    public boolean isPreviewRunning() {
        return mPreviewRunning;
    }

    private synchronized void releaseCamera() {
        // 이전 카메라 초기화
        if (mListener != null) {
            mListener.onStopCamera();
        }

        if (mCameraInstance != null) {
            try {
                mCameraInstance.setPreviewCallbackWithBuffer(null);
            } catch (Exception e) {
                // Does nothing
            }
        }

        if (mPreviewBuffer!=null) {
            if (mPreviewBuffer[mCurrentCameraId] != null) {
                // 이전 카메라 버퍼가 있으면 지운다
                mPreviewBuffer[mCurrentCameraId].releaseBuffer();
            }
        }


//        if (mCameraOpenThread != null) {
//            if (!mCameraOpenThread.isInterrupted() && mCameraOpenThread.isAlive()) {
//                mCameraOpenThread.interrupt();
//                try {
//                    mCameraOpenThread.join();
//                } catch (InterruptedException e) {
//                    // Does nothing
//                }
//            }
//        }

        if (mCameraInstance != null) {
            // 이전 카메라 삭제
            try {
                Log.i("CAMERA MANGER", "[[[[[ release camera and set preview callback null 2");
                mCameraInstance.setPreviewCallbackWithBuffer(null);
                mCameraInstance.release();
            } catch (Exception e) {
                // Does nothing
            }
            mCameraInstance = null;
        }
    }

    public void stop() {
        if (mListener != null && mPreviewRunning) {
            mListener.onStopCamera();
        }

        releaseCamera();
    }

    public int switchCamera() {
        mOpenCameraId = -1;
        if (mListener!=null) {
            mListener.onSwitchCameraStarted();
        }

        return startPreview(getNextCameraId(mCurrentCameraId));
    }

    private int getNextCameraId(int cameraId) {
        return (cameraId+1)%mCameraHelper.getNumberOfCameras();
    }

    private int getPrevCameraId(int cameraId) {
        return (cameraId-1+mCameraHelper.getNumberOfCameras())%mCameraHelper.getNumberOfCameras();
    }


    public void render() {
        // 원래 mDrawCameraId를 바꿔가며 face detection 및 bilateral 하던건데. 필요 없을 것 같아 다 지움..
    }

    private static native void nativeOnFrameReceived(final byte[] data, int arraySize, int previewWidth, int previewHeight);

    private static native void nativeOnCodeDetected(float[] pts, int type, String detectString);


    @Override
    public synchronized void onPreviewFrame(byte[] data, Camera camera) {

        if (data != null) {
            //Log.i("CAMERA MANASGER", "[[[[[ preview frame 1 : " + Integer.toString(data.length));
//            YuvImage image = new YuvImage(data, ImageFormat.NV21, mPreviewBuffer[mCurrentCameraId].getPreviewWidth(), mPreviewBuffer[mCurrentCameraId].getPreviewHeight(), null);
//            Rect rectangle = new Rect();
//            rectangle.bottom = mPreviewBuffer[mCurrentCameraId].getPreviewHeight();
//            rectangle.top = 0;
//            rectangle.left = 0;
//            rectangle.right = mPreviewBuffer[mCurrentCameraId].getPreviewWidth();
//            ByteArrayOutputStream out2 = new ByteArrayOutputStream();
//            image.compressToJpeg(rectangle, 100, out2);
//            byte[] byteBuffer = out2.toByteArray();
//            if (byteBuffer==null) {
//                Log.i("CAMERA MANAGER", "[[[[[ image compress error !!!!");
//            }

//            Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);


            if (isScannerSupport) {

                ByteBuffer bf = ByteBuffer.wrap(data);

                if (detector.isOperational() && bf != null) {
                    Frame frame = new Frame.Builder().setImageData(bf, mPreviewBuffer[mCurrentCameraId].getPreviewWidth(), mPreviewBuffer[mCurrentCameraId].getPreviewHeight(), 842094169).build();
                    SparseArray<Barcode> barcodes = detector.detect(frame);
                    for (int index = 0; index < barcodes.size(); index++) {
                        Barcode code = barcodes.valueAt(index);
                        int type = barcodes.valueAt(index).valueFormat;

//                        String tmp = "";
                        Point[] pts = code.cornerPoints;
                        float[] fpts = new float[8];
//                        int ncount = 0;
                        // tl, tr, rb, lb
                        for (int i=0; i<pts.length; i++) {
                            Point pt = pts[i];
                            fpts[i*2] = pt.x;
                            fpts[i*2+1] = mPreviewBuffer[mCurrentCameraId].getPreviewHeight() - pt.y;
//                            tmp = tmp + ", [" + pt.x + "/" + pt.y + "]";
                        }

//                        Log.i("TEST", "[[[[[ Detect code : " + tmp);

//                        Log.i("TEST", "[[[[[ detect code rawValue : " + code.rawValue.toString() + ", type : " + type + ", stringValue : " + code.displayValue.toString());

                        //Required only if you need to extract the type of barcode

                        String resultString = code.displayValue.toString();
//                        Log.i("TEST", "[[[[[ Detected string : " + resultString);


                        DeviceCameraManager.nativeOnCodeDetected(fpts, type, resultString);

                        break;
//                        DeviceCameraManager.nativeOnCodeDetected(code.rawValue);

//                        String resultString = "";
//
//                        switch (type) {
//                            case Barcode.CONTACT_INFO:
////                                Log.i("TEST", "[[[[[ detect scan code!!! contact : " + code.contactInfo.title);
////                                DeviceCameraManager.nativeOnCodeDetected(code.contactInfo.title);
//                                resultString = code.contactInfo.title;
//                                DeviceCameraManager.nativeOnCodeDetected(resultString);
//                                break;
//                            case Barcode.EMAIL:
////                                Log.i("TEST", "[[[[[ detect scan code!!! email : " + code.email.address);
////                                DeviceCameraManager.nativeOnCodeDetected(code.email.address);
//                                resultString = code.email.address;
//                                DeviceCameraManager.nativeOnCodeDetected(resultString);
//                                break;
//                            case Barcode.ISBN:
////                                Log.i("TEST", "[[[[[ detect scan code!!! isbn : " + code.rawValue);
////                                DeviceCameraManager.nativeOnCodeDetected(code.rawValue);
//                                resultString = code.rawValue;
////                                DeviceCameraManager.nativeOnCodeDetected(resultString);
//                                Log.i("TEST", "[[[[[ detect scan code!!! isbn : " + code.rawValue);
//                                break;
//                            case Barcode.PHONE:
////                                Log.i("TEST", "[[[[[ detect scan code!!! phone : " + code.phone.number);
////                                DeviceCameraManager.nativeOnCodeDetected(code.phone.number);
//                                resultString = code.phone.number.toString();
//                                DeviceCameraManager.nativeOnCodeDetected(resultString);
//                                break;
//                            case Barcode.PRODUCT:
////                                Log.i("TEST", "[[[[[ detect scan code!!! product : " + code.rawValue);
////                                DeviceCameraManager.nativeOnCodeDetected(code.rawValue);
//                                resultString = code.rawValue;
//                                DeviceCameraManager.nativeOnCodeDetected(resultString);
//                                break;
//                            case Barcode.SMS:
////                                Log.i("TEST", "[[[[[ detect scan code!!! sms : " + code.sms.message);
////                                DeviceCameraManager.nativeOnCodeDetected(code.sms.message);
//                                resultString = code.sms.message;
//                                DeviceCameraManager.nativeOnCodeDetected(resultString);
//                                break;
//                            case Barcode.TEXT:
////                                Log.i("TEST", "[[[[[ detect scan code!!! text : " + code.rawValue);
////                                DeviceCameraManager.nativeOnCodeDetected(code.rawValue);
//                                resultString = code.rawValue;
//                                DeviceCameraManager.nativeOnCodeDetected(resultString);
//                                break;
//                            case Barcode.URL:
////                                Log.i("TEST", "[[[[[ detect scan code!!! URL : " + code.url.url);
////                                DeviceCameraManager.nativeOnCodeDetected(code.url.url);
//                                resultString = code.url.url;
//                                DeviceCameraManager.nativeOnCodeDetected(resultString);
//                                break;
//                            case Barcode.WIFI:
////                                Log.i("TEST", "[[[[[ detect scan code!!! WiFi : " + code.wifi.ssid);
////                                DeviceCameraManager.nativeOnCodeDetected(code.wifi.ssid);
//                                resultString = code.wifi.ssid;
//                                DeviceCameraManager.nativeOnCodeDetected(resultString);
//                                break;
//                            case Barcode.GEO:
////                                Log.i("TEST", "[[[[[ detect scan code!!! : GoePoint lat : " + code.geoPoint.lat + ", lng : " + code.geoPoint.lng);
////                                DeviceCameraManager.nativeOnCodeDetected();
////                                resultString = code.geoPoint.lat.toString() + "," + code.geoPoint.lng.toString();
//                                resultString = "";
//                                break;
//                            case Barcode.CALENDAR_EVENT:
////                                Log.i("TEST", "[[[[[ detect scan code!!! Calendar event : " + code.calendarEvent.description);
////                                DeviceCameraManager.nativeOnCodeDetected();
//                                resultString = code.calendarEvent.description;
//                                break;
//                            case Barcode.DRIVER_LICENSE:
////                                Log.i("TEST", "[[[[[ detect scan code!!! Driver License : " + code.driverLicense.licenseNumber);
////                                DeviceCameraManager.nativeOnCodeDetected();
//                                resultString = code.driverLicense.licenseNumber.toString();
//                                break;
//                            default:
////                                Log.i("TEST", "[[[[[ detect scan code!!! Unkown : " + code.rawValue);
////                                DeviceCameraManager.nativeOnCodeDetected();
//                                resultString = code.rawValue;
//                                break;
//                        }
//
////                        Log.i("TEST", "DETECTED STRING : " + resultString);
////                        DeviceCameraManager.nativeOnCodeDetected(resultString);
//                        // only one detected
//                        break;
                    }


//                    if (barcodes.size() == 0) {
//                        Log.i("TEST", "[[[[[ not detected code!!!");
//                    }
                }
            }

            //Log.i("CAMERA MANASGER", "[[[[[ preview frame (JAVA) : " + Integer.toString(data.length));
            if (mPreviewBuffer !=null && mPreviewBuffer[mCurrentCameraId] != null) {
                //Log.i("CAMERA MANAGER", "[[[[[ preview frame : " + Integer.toString(mCurrentCameraId) + ", (" + Integer.toString(mPreviewBuffer[mCurrentCameraId].getPreviewWidth()) + ", " + Integer.toString(mPreviewBuffer[mCurrentCameraId].getPreviewHeight()) + ")");
                DeviceCameraManager.nativeOnFrameReceived(data, data.length, mPreviewBuffer[mCurrentCameraId].getPreviewWidth(), mPreviewBuffer[mCurrentCameraId].getPreviewHeight());
            }
            camera.addCallbackBuffer(data);
            //Log.i("CAMERA MANASGER", "[[[[[ preview frame 2 : " + Integer.toString(data.length));
        }
    }

    private Camera getCameraInstance(final int id) {
        Camera camera = null;
        try {
            camera = mCameraHelper.openCamera(id);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return camera;
    }

    private void setPreviewSetting() {

        final int width, height;

        CameraInfo2 cameraInfo = new CameraInfo2();
        Log.i("CAMERA MANAGER", "[[[[[ set Preview Setting new Camera ID : " + Integer.toString(mCurrentCameraId));
        mCameraHelper.getCameraInfo(mCurrentCameraId, cameraInfo);

        Parameters params = mCameraInstance.getParameters();

        final int reqPreviewWidth;
        final int reqPreviewHeight;

        if (cameraInfo.facing ==  CameraInfo.CAMERA_FACING_FRONT) {
            // 4:3 화면
            reqPreviewWidth = (int)mWidth;
            reqPreviewHeight = (int)(4*mWidth/3);
            Log.i("CAMAERA MANGER", "[[[[[ req preview (4:3) width : " + Integer.toString(reqPreviewWidth) + ", height : " + Integer.toString(reqPreviewHeight));
        } else {
            // 전체화면
            reqPreviewWidth = mWidth;
            reqPreviewHeight = mHeight;
            Log.i("CAMAERA MANGER", "[[[[[ req preview (full) width : " + Integer.toString(reqPreviewWidth) + ", height : " + Integer.toString(reqPreviewHeight));
        }


        Size previewSize = CameraUtil.getOptimalPreviewSize(mCurrentCameraId, params, reqPreviewWidth, reqPreviewHeight);
        Log.i("CAMAERA MANGER", "[[[[[ new preview width : " + Integer.toString(previewSize.width) + ", height : " + Integer.toString(previewSize.height));
        if (cameraInfo.facing ==  CameraInfo.CAMERA_FACING_FRONT) {
            Log.i("CAMERA MANAGER", "[[[[[ CAMERA CALL PREVIEW FOR FRONT 1 !!!!!!");
            if ((float)previewSize.width / previewSize.height > 1.5f) {
                Log.i("CAMERA MANAGER", "[[[[[ CAMERA CALL PREVIEW FOR FRONT 1-1 !!!!!!");
                Size candicateSize = CameraUtil.getOptimalPreviewSize(mCurrentCameraId, params, 600, 800);
                if (candicateSize.width == 800 && candicateSize.height == 600) {
                    previewSize = candicateSize;
                    Log.i("CAMERA MANAGER", "[[[[[ CAMERA CALL PREVIEW FOR FRONT 1-1-1 !!!!!!");
                } else {
                    candicateSize = CameraUtil.getOptimalPreviewSize(mCurrentCameraId, params, 480, 640);
                    Log.i("CAMERA MANAGER", "[[[[[ CAMERA CALL PREVIEW FOR FRONT 1-2-1 !!!!!!");
                    if (candicateSize.width == 640 && candicateSize.height == 480) {
                        previewSize = candicateSize;
                        Log.i("CAMERA MANAGER", "[[[[[ CAMERA CALL PREVIEW FOR FRONT 1-2-2 !!!!!!");
                    }
                }
            }
        } else {
            Log.i("CAMERA MANAGER", "[[[[[ CAMERA CALL PREVIEW FOR BACK!!!!!!");
        }

        Log.i("CAMERA MANAGER", "[[[[[ get camea preview size @@@@@ Width : " + Integer.toString(previewSize.width) + ", Height : " + Integer.toString(previewSize.height));

        //int pixelFormat = params.getPreviewFormat();
        //Log.i("CAMERA MANAGER", "[[[[[ pixel format 1 : " + Integer.toString(pixelFormat));
        //params.setPreviewFormat(ImageFormat.JPEG);
        //Log.i("CAMERA MANAGER", "[[[[[ pixel format 2 : " + Integer.toString(pixelFormat));

        int frameRate = params.getPreviewFrameRate();
//        params.setPreviewFrameRate(10);
        Log.i("CAMERA MANAGER", "[[[[[ frame rate : " + Integer.toString(frameRate));
        params.setPreviewSize(previewSize.width, previewSize.height);

        Size pictureSize = CameraUtil.getOptimalPictureSize(params, previewSize);
        Log.i("CAMERA MANAGER", "[[[[[[[[[[[[[[[ support picture size width : " + Integer.toString(pictureSize.width) + ", height : " + Integer.toString(pictureSize.height));
        params.setPictureSize(pictureSize.width, pictureSize.height);

        width = params.getPreviewSize().width;
        height = params.getPreviewSize().height;
        Log.i("CAMERA MANAGER", "[[[[[[[[[[[[[[[[[[[[ PREVIE SIZE(" + Integer.toString(mCurrentCameraId) + ") :  (" + Integer.toString(width) + ", " + Integer.toString(height) + ")");
        params.setJpegQuality(100);

        if (mCameraHelper.isSupportContinuousFocus()) {
            mCameraHelper.setFocusModeContinuous(params);
            // 이거 분기 처리 해야하나나??
            //camera.stAutoFocusMoveCallback(mAutofocusMoveCallback);
        }

        if (mCameraHelper.isSupportZoom()) {
            mCameraHelper.setZoomValue(params, 0);
        }

        if (mCameraHelper.isSupportAutoWhiteBalance()) {
            params.setWhiteBalance(Parameters.WHITE_BALANCE_AUTO);
        }

        if (mCameraHelper.isSupportAutoSceneMode()) {
            params.setSceneMode(Parameters.SCENE_MODE_AUTO);
        }

        // 여기서 face detection을 했으나.. 지금은 안쓴다
//        List<Integer> formats = params.getSupportedPreviewFormats();
//        for (Integer format : formats) {
//            Log.i("CAMERA MANAGER", "[[[[[ supported preview formats : " + format.toString());
//        }

        params.setPreviewFormat(ImageFormat.NV21);
        //params.setPreviewFrameRate(10);
        mCameraInstance.setParameters(params);

        final int orientation = mCameraHelper.getCameraDisplayOrientation(mActivity, mCurrentCameraId);
        final boolean isFrontfacing = cameraInfo.facing == CameraInfo.CAMERA_FACING_FRONT ? true : false;

        // 이거 없이도 preview frame이 넘어올까???
//                try {
//                    camera.setPreviewDisplay(mSurfaceView.getHolder());
//                } catch (IOException e) {
//                    throw new InterruptedException("Preview Display Failed");
//                }

        //mCameraHelper.setCameraDisplayOrientation(mActivity, mCurrentCameraId, mCameraInstance);

        mOpenWidth = width;
        mOpenHeight = height;
        mOpenOrientation = orientation;
        mOpenFrontFacing = isFrontfacing;
        mOpenCameraId = mCurrentCameraId;

        Log.i("CAMERA MANGER", "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ preview Width : " + Integer.toString(mOpenWidth) + ", Height : " + Integer.toString(mOpenHeight) + " ]]]]]]]]]]]]]]]]]]");

    }

    private int startPreview(int cameraId) {
        Log.i("CAMERA MANGER", "[[[[[ start Preview oldCameraId :" + Integer.toString(mCurrentCameraId) + ", newCameraId : " + Integer.toString(cameraId));
        releaseCamera();
        // 새 카메라 ID로 교체
        mCurrentCameraId = cameraId;
        mCameraInstance = getCameraInstance(mCurrentCameraId);
        setPreviewSetting();

        try {
            if (mPreviewBuffer[mCurrentCameraId] == null) {
                mPreviewBuffer[mCurrentCameraId] = CameraPreviewBuffer.createPreviewBuffer(mOpenHeight, mOpenWidth, mOpenWidth, mOpenHeight, mOpenOrientation, mOpenFrontFacing?true:false, false);
            }

            for (int i=0; i<CameraPreviewBuffer.NUM_PREVIEW_BUFFER; i++) {
                mCameraInstance.addCallbackBuffer(mPreviewBuffer[mCurrentCameraId].getDataBuffer(i));
            }
            mCameraInstance.setPreviewCallbackWithBuffer(this);
            Log.i("CAMERA MANAGER", "[[[[[ start Preview 2");
        } catch (Exception e) {
            Log.i("CAMERA MANAGER", "[[[[[ start Preview exceptions!!!!!!");
            return 90;
        }


        mCameraInstance.startPreview();

        return mOpenOrientation;

//        mCameraOpenThread = new CameraOpenThread(cameraId);
//        mCameraOpenThread.start();
    }

    public int startCameraPreview() {
        return startPreview(mCurrentCameraId);
    }

    public void capture() {
//        if (!isPreviewRunning())
//            return;

        Log.i("CAMERA MANAGER", "[[[[[ capture!!!!!! capture!!!!!");
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                synchronized (DeviceCameraManager.this) {
                    Log.i("CAMERA MANGER", "[[[[[ capture camera and set preview callback null");
                    mCameraInstance.setPreviewCallbackWithBuffer(null);
                    mCameraInstance.takePicture(DeviceCameraManager.this, null, DeviceCameraManager.this);
                }
            }
        });
    }

    private static native void nativeOnPictureTaken(final byte[] data, int arraySize, int width, int height);

    @Override
    public void onPictureTaken(byte[] data, Camera camera) {
        // byte[] data를 바로 전달하자..
        ByteArrayInputStream is = new ByteArrayInputStream(data);
        BitmapFactory.Options options = new BitmapFactory.Options();

        try {
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeStream(is, null, options);
            is.close();
            is = null;
        } catch (Exception e) {
            Log.i("CAMERA MANAGER", "[[[[[ onPictureTaken... exception... occured!!!");
            return;
        }

        int outWidth  = options.outWidth;
        int outHeight = options.outHeight;

        Log.i("CAMERA MANGER", "[[[[[ onPicture taken!!!!! from java");
        DeviceCameraManager.nativeOnPictureTaken(data, data.length, outWidth, outHeight);

//        return;
//        if (mListener!=null) {
//            mListener.onPictureTaken(data);
//        }
//        if (mPreviewBuffer !=null && mPreviewBuffer[mCurrentCameraId] != null) {
//            //Log.i("CAMERA MANAGER", "[[[[[ preview frame : " + Integer.toString(mCurrentCameraId) + ", (" + Integer.toString(mPreviewBuffer[mCurrentCameraId].getPreviewWidth()) + ", " + Integer.toString(mPreviewBuffer[mCurrentCameraId].getPreviewHeight()) + ")");
//            DeviceCameraManager.nativeOnFrameReceived(data, data.length, mPreviewBuffer[mCurrentCameraId].getPreviewWidth(), mPreviewBuffer[mCurrentCameraId].getPreviewHeight());
//        }

    }


    @Override
    public void onShutter() {
        Log.i("SHUTTER", "[[[[[ take a picture by shutter  !!!!");
    }

    // 외부에서 focus를 호출 했다... 따라서 카메라가 포커싱 동작을 해야한다
    public boolean startAutoFocus(float focusMapX, float focusMapY) {
        if (isPreviewRunning() && mCameraHelper.isSupportAutoFocus()) {
            try {
                boolean updateParams = false;
                Parameters params = mCameraInstance.getParameters();
                if (!params.getFocusMode().equals(Camera.Parameters.FOCUS_MODE_AUTO)) {
                    mCameraHelper.setFocusModeAuto(params);
                    updateParams = true;
                }

                if (updateParams) {
                    //params.setPreviewFormat(ImageFormat.YUY2);
                    //params.setPreviewFrameRate(10);
                    mCameraInstance.setParameters(params);
                }

                mCameraInstance.autoFocus(mAutofocusCallback);
                return true;

            } catch (Exception e) {
                e.printStackTrace();
            } catch (Error e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    // 아마 이거 지원 안 할 듯...
    public boolean isSupportContinuousFocus() {
        return mCameraHelper.isSupportContinuousFocus();
    }
    public void startContinuousFocus() {
        if (isPreviewRunning() && mCameraHelper.isSupportContinuousFocus()) {
            try {
                mCameraInstance.cancelAutoFocus();

                Parameters params = mCameraInstance.getParameters();

                if (mCameraHelper.isSupportFocusArea() || mCameraHelper.isSupportMeteringArea()) {
                    Rect area = new Rect(-100, -100, 100, 100);

                    List<Camera.Area> focusAreas = new ArrayList<>();
                    focusAreas.add(new Camera.Area(area, 1000));

                    if (mCameraHelper.isSupportFocusArea()) {
                        params.setFocusAreas(focusAreas);
                    }
                    if (mCameraHelper.isSupportMeteringArea()) {
                        params.setMeteringAreas(focusAreas);
                    }
                }
                mCameraHelper.setFocusModeContinuous(params);
                //params.setPreviewFormat(ImageFormat.YUY2);
                //params.setPreviewFrameRate(10);
                mCameraInstance.setParameters(params);

            } catch (Exception e) {
                e.printStackTrace();
            } catch (Error e) {
                e.printStackTrace();

            }
        }
    }

    private int mZoomValue = 0;

    public synchronized void setZoomValue(int zoomValue) {
        try {
            Parameters params = mCameraInstance.getParameters();
            int currentZoomValue = mZoomValue;//params.getZoom();

            if (currentZoomValue != zoomValue) {
                mZoomValue = zoomValue;
                params.setZoom(zoomValue);
                //params.setPreviewFormat(ImageFormat.YUY2);
                //params.setPreviewFrameRate(10);
                mCameraInstance.setParameters(params);
            }

        } catch (Exception e) {
            e.printStackTrace();
        } catch (Error e) {
            e.printStackTrace();
        }

    }

    public CameraCaps getCameraCaps() {
        return mCameraHelper.getCameraCaps();
    }


    void mapPreviewPoint(float x, float y, PointF outPoint) {

        float scale;
        if (mOpenOrientation == 90 || mOpenOrientation == 270) {
            scale = mOpenHeight/mWidth;
        } else {
            scale = mOpenWidth/mHeight;
        }

        x -= mPreviewPosX;
        y -= mPreviewPosY;

        if (mOpenOrientation == 90 || mOpenOrientation == 270) {
            float tmp = x;
            x = y;
            y = tmp;
        }

        if (mOpenFrontFacing) {
            y *= -1;
        }

        x =  1000f * x * scale / (0.5f*mOpenWidth);
        y = -1000f * y * scale / (0.5f*mOpenHeight);

        x = Math.min(Math.max(-1000, x), 1000);
        y = Math.min(Math.max(-1000, y), 1000);

        if (outPoint != null) {
            outPoint.x = x;
            outPoint.y = y;
        }
    }

    public boolean isFrontFacing() {
        return mOpenFrontFacing;
    }

    public boolean isAutoFocusSupported() {
        return mCameraHelper.isSupportAutoFocus();
    }

    public boolean hasFrontCamera() {return mCameraHelper.hasFrontCamera();}
    public boolean hasBackCamera() {return mCameraHelper.hasBackCamera();}
    public boolean hasFlash() {return mCameraHelper.isSupportFlash();}


}