/****************************************************************************
Copyright (c) 2015-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
package org.cocos2dx.cpp;


import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.location.Address;
import android.location.Geocoder;
import android.location.Location;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Handler;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.FragmentActivity;
import android.util.Log;

//import android.support.v4.app.ActivityCompat;
//import android.support.v4.content.ContextCompat;
//import android.support.v4.content.FileProvider;

import android.support.v4.content.FileProvider;
import android.view.View;
import android.webkit.WebView;


import com.brokenpc.SMCocosLib.CallbackLocation;
import com.brokenpc.SMCocosLib.DeviceCameraManager;
import com.brokenpc.SMCocosLib.DeviceImageManager;
import com.brokenpc.SMCocosLib.GPSManager;
import com.brokenpc.SMCocosLib.OnPhoneImagesObtained;
import com.brokenpc.SMCocosLib.PhoneAlbum;
import com.brokenpc.SMCocosLib.PhonePhoto;

import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxHelper;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.List;
import java.util.Locale;
import java.util.Vector;

public class AppActivity extends Cocos2dxActivity implements OnPhoneImagesObtained  {

    private final int REQ_ACCESS_LOCATION = 1001;
    private final int REQ_MEDIA = 1002;
    private final int REQ_NFC = 1003;

    private static int JNI_TEST_STRING = 1;
    private static AppActivity  mActivity = null;
    private static Bitmap bmp = null;
    private static int DEFAULT_SIDE_LENGTH = 256;
    private static int MAX_SIDE_LENGTH = 1280;
    private static Vector<PhoneAlbum> phoneAlbums;
    private double dLatitude = 0.0;
    private double dLongitude = 0.0;
    private String strAddress = "";

    private boolean canAccessGPS =  false;
    private boolean canAccessAlbum = false;
    private boolean canAccessCamera = false;
    private boolean canAccessPhone = false;
    private boolean canAccessContact = false;


    public static DeviceCameraManager deviceCameraManager;

    private GPSManager gpsManager = null;


    public void onLoadAlbumComplete(Vector<PhoneAlbum> phoneAlbums) {
        this.phoneAlbums = phoneAlbums;
        for (PhoneAlbum album : this.phoneAlbums) {
            Log.i("ALBUM INFO", album.getName() + " : " + Integer.toString(album.getPhotoCount()) + "==" + Integer.toString(album.getAlbumPhotos().size()));
        }
    }


    public void onError() {

    }

    @Override
    public void onCreate(Bundle savedInstaceState)
    {
        super.onCreate(savedInstaceState);


        if (needGPSPermission()) {
            return;
        }

        gpsManager = new GPSManager(this, callbackEvent);
        gpsManager.startGPS();

        if (needRWPermission()) {
            return;
        }



        mActivity = this;

        DeviceImageManager.getPhoneAlbumInfo(this.getApplicationContext(), this);


        deviceCameraManager = new DeviceCameraManager(this, this.getWindow().getDecorView().getWidth(), this.getWindow().getDecorView().getHeight(), null
                , null);

//        checkForUpdates();
        getExternalScheme(getIntent());
    }

    @Override
    public void onBackPressed() {
        Log.i("APP", "[[[[[ onBackPressed");
//        super.onBackPressed();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        getExternalScheme(intent);
    }

    public void getExternalScheme(Intent intent) {
        String type = null;

        Uri uri = intent.getData();
        if (uri!=null) {
            String scheme = uri.getScheme();
            String host = uri.getHost();
            String data = uri.toString();
            Log.i("scheme", "[[[[[ uri : " + data);
            AppActivity.nativeGetScheme(data);
        }
    }

    private static native void nativeGetScheme(final String scheme);

    @Override
    public void onResume() {
        super.onResume();
//        checkForCrashes();
    }

    @Override
    public void onPause() {
        super.onPause();;
//        unregisterManagers();
    }

//    @Override
//    public void onDestroy() {
//        super.onDestroy();;
//        unregisterManagers();
//    }

//    private void checkForCrashes() {
//        CrashManager.register(this);
//    }
//
//    private void checkForUpdates() {
//        UpdateManager.register(this);
//    }
//
//    private void unregisterManagers() {
//        UpdateManager.unregister();
//    }


    private boolean needGPSPermission() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.ACCESS_COARSE_LOCATION},
                    REQ_ACCESS_LOCATION);

            canAccessGPS = false;
            return true;
        } else {
            canAccessGPS = true;
            return false;
        }
    }

    private boolean needRWPermission() {
        if ( ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                && ActivityCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                && ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA},
                    REQ_MEDIA);
            canAccessAlbum = false;
            return true;
        } else {
            canAccessAlbum = true;
            return false;
        }
    }


    public static void initCamera(int cameraId) {
        deviceCameraManager.initCameraOnce(cameraId);
    }

    public static int cameraPreviewStart() {
        return deviceCameraManager.startCameraPreview();
    }

    public static void stopCamera() {
        deviceCameraManager.stop();
    }

    public static boolean hasFrontCamera() {
        return true;
    }

    public static int switchCamera() {
        Log.i("SMIMAGEEDITOR", "[[[[[ call switch camera JNI from Cocos2d");
        return deviceCameraManager.switchCamera();
    }

    public static void capture() {
        Log.i("SMIMAGEEDITOR", "[[[[[ call capture JNI from Cocos2d");
        deviceCameraManager.capture();
    }

    public static boolean hasFlash() {
        return deviceCameraManager.hasFlash();
    }

    //private static native void nativeFinish();

    public void closeCocos() {

        Intent intent = new Intent();
        intent.putExtra("JUST_CLOSE", true);
        setResult(RESULT_OK, intent);
        finish();;
    }

    public static void closeCocosActivity() {
        if(mActivity == null)
            return;
        mActivity.closeCocos();
    }

    // for scan code
    static public void setCodeScanSupport(boolean bSet) {
        deviceCameraManager.setScanSupport(bSet);
    }

    // device id

    static public String getUUID()
    {
        return "test";
    }

    static public String getUDID()
    {
        return "test";
    }

    static public String getGroupCoverUrl2(int groupIndex) {
        return "test";
    }

    static public boolean isAccesibleGPS() {
        if (ActivityCompat.checkSelfPermission(mActivity, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(mActivity, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            return false;
        } else {
            return true;
        }
    }

    private static native void nativeOnAskPermissionComplete(final int requestCode, boolean bAccessible);

//    static public void startGPS() {
//        Log.i("TEST","[[[[[ startGPS 1 ");
//        if (mActivity.gpsManager==null) {
//            Log.i("TEST","[[[[[ startGPS 1-1 ");
//            mActivity.gpsManager = new GPSManager(mActivity, mActivity.callbackEvent);
//        }
//        Log.i("TEST","[[[[[ startGPS 2 ");
//        if (mActivity.gpsManager != null) {
//            Log.i("TEST","[[[[[ startGPS 2-1 ");
//            mActivity.gpsManager.startGPS();
//        }
//        Log.i("TEST","[[[[[ startGPS 3 ");
//    }
//
//    static public void stopGPS() {
//        if (mActivity.gpsManager != null) {
//            mActivity.gpsManager.stopGPS();
//        }
//    }


    static public void askPermissionGPS() {

        if (ActivityCompat.checkSelfPermission(mActivity, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(mActivity, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            // 권한이 없으면 물어보자...
            ActivityCompat.requestPermissions(mActivity, new String[]{Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.ACCESS_COARSE_LOCATION}, mActivity.REQ_ACCESS_LOCATION);
        } else {
            AppActivity.nativeOnAskPermissionComplete(mActivity.REQ_ACCESS_LOCATION, true);
        }
    }

    static public float getLatitude() {
        return (float)mActivity.dLatitude;
    }

    static public float getLongitude() {
        return (float)mActivity.dLongitude;
    }

    static public String getLocationName() {
        return  mActivity.strAddress;
    }

    private static native void nativeGetLocaitonInfoLoadComplete();


    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {

        switch (requestCode) {
            case REQ_ACCESS_LOCATION:
                boolean bResult =  false;
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    bResult = true;
//                    if (gpsManager==null) {
//                        new GPSManager(mActivity, mActivity.callbackEvent);
//                    }
                    canAccessGPS = true;
                } else {
//                    bResult = false;
//                    finish();
                    canAccessGPS = false;
                }
                // jni 로 보내자
//                AppActivity.nativeOnAskPermissionComplete(requestCode, (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED));

                if (canAccessAlbum==false) {
                    needRWPermission();
                }

                return;
            case REQ_MEDIA:
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    canAccessAlbum = true;
                } else {
                    canAccessAlbum = false;
                }
                if (canAccessGPS==false) {
                    needGPSPermission();
                }
                return;
            case REQ_NFC:
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                } else {
                }
                return;
        }
    }

    private CallbackLocation callbackEvent = new CallbackLocation() {
        @Override
        public void callbackMethod(Location location) {

//            Log.i("TEST", "callbackMethod~callbackMethod~callbackMethod~");


            //TODO : Location 데이터 획득 후 처리
            if (location != null) {
                dLatitude = location.getLatitude();
                dLongitude = location.getLongitude();

                Geocoder gCoder = new Geocoder(getApplicationContext(), Locale.getDefault());
                try {
                    List<Address> addrList = gCoder.getFromLocation(dLatitude, dLongitude, 1);
                    StringBuilder strBlder = new StringBuilder();
                    if (addrList.size() > 0) {
                        Address addr = addrList.get(0);
                        for (int i = 0; i < addr.getMaxAddressLineIndex(); i++)
                            if(addr.getAddressLine(i) != null) {
                                strBlder.append(addr.getAddressLine(i) + "\n");
                            }
                        if(addr.getAdminArea() != null ) {
                            strBlder.append(addr.getAdminArea()).append(" ");
                        }
                        if(addr.getLocality() != null ) {
                            strBlder.append(addr.getLocality()).append(" ");
                        }
                        if(addr.getThoroughfare() != null ) {
                            strBlder.append(addr.getThoroughfare()).append(" ");
                        }
                        if(addr.getSubThoroughfare() != null ) {
                            strBlder.append(addr.getSubThoroughfare());
                        }
//		                    strBlder.append(addr.getAdminArea()).append("\n");
//		                    strBlder.append(addr.getLocality()).append("\n");
//		                    strBlder.append(addr.getThoroughfare()).append("\n");
//		                    strBlder.append(addr.getSubThoroughfare()).append("\n");
//                        strBlder.append(addr.getAdminArea()).append(" ");
//                        strBlder.append(addr.getLocality()).append(" ");
//                        strBlder.append(addr.getThoroughfare()).append(" ");
//                        strBlder.append(addr.getSubThoroughfare());
//		                    strBlder.append(addr.getCountryName());
                        strAddress = strBlder.toString();
                    } else
                        strAddress = "Can't find Address";
                } catch (IOException e) {
//					System.out.println("IO Exception Occurs\n");
                }

//                AppActivity.nativeGetLocaitonInfoLoadComplete();


            } else {
                // GPS가 꺼져있는 경우 location이 null로 내려오므로 설정화면으로 보내서 GPS를 켜도록 함
            }
        }
    };


    // get group count
    static public int getGroupCount() {
        return mActivity.phoneAlbums.size();
    }


    // loop를 돌지 않기 위해 수정.
    // group 및 photo를 ID로 가져오려면 loop를 돌아야 하는데
    // 이미 갖추어진 Array 이므로 그냥 index로만 access하기로 함. 2017-07-10

    // get group cover photo url by group index
    static public String getGroupCoverUrl(int groupIndex) {
        if (groupIndex>=mActivity.phoneAlbums.size())
            return "";

        PhoneAlbum phoneAlbum = mActivity.phoneAlbums.get(groupIndex);
        return phoneAlbum.getCoverUri();
    }

    // get group name by group index
    static public String getGroupName(int groupIndex) {
        if (groupIndex>=mActivity.phoneAlbums.size() )
            return "";

        PhoneAlbum phoneAlbum = mActivity.phoneAlbums.get(groupIndex);
        return phoneAlbum.getName();
    }

    // get photo count in group
    static public int getPhotoCount(int groupIndex) {
        if (groupIndex>=mActivity.phoneAlbums.size())
            return 0;

        PhoneAlbum phoneAlbum = mActivity.phoneAlbums.get(groupIndex);

        return phoneAlbum.getPhotoCount();

        /*
        int photoCount = phoneAlbum.getAlbumPhotos().size();

        if (photoCount==0) {
            photoCount = DeviceImageManager.getPhotosInfo(mActivity.getApplicationContext(), phoneAlbum).getAlbumPhotos().size();
            Log.i("GETPHOTOCOUNT", "[[[[[ get photo count in device manager (" + Integer.toString(phoneAlbum.getId()) + ") : " + Integer.toString(photoCount));
        }

        Log.i("GETPHOTOCOUNT", "[[[[[ get photo count (" + Integer.toString(phoneAlbum.getId()) + ") : " + Integer.toString(photoCount));
        return photoCount;
        */
    }
/*
    // get photo ID --> 이게 쓸일이 있을까?
    static public int getPhotoID(int groupIndex, int photoIndex) {
        if (groupIndex>=mActivity.phoneAlbums.size())
            return 0;

        return;

        PhoneAlbum phoneAlbum = mActivity.phoneAlbums.get(groupIndex);
        if (photoIndex>=phoneAlbum.getAlbumPhotos().size())
            return 0;

        PhonePhoto phonePhoto = phoneAlbum.getAlbumPhotos().get(photoIndex);
        return phonePhoto.getId();
    }
*/

    // get photo orientation
    static public int getPhotoOrientation(int groupIndex, int photoIndex) {
        if (groupIndex>=mActivity.phoneAlbums.size())
            return 0;

        PhoneAlbum phoneAlbum = mActivity.phoneAlbums.get(groupIndex);

        if (phoneAlbum.getAlbumPhotos().size()!=phoneAlbum.getPhotoCount()) {
            phoneAlbum.getAlbumPhotos().clear();;
            phoneAlbum = DeviceImageManager.getPhotosInfo(mActivity.getApplicationContext(), phoneAlbum);
        }


        if (photoIndex>=phoneAlbum.getAlbumPhotos().size())
            return 0;

        PhonePhoto phonePhoto = phoneAlbum.getAlbumPhotos().get(photoIndex);
        return phonePhoto.getOrientation();
    }

    // get photo url
    static public String getPhotoUrl(int groupIndex, int photoIndex) {
        if (groupIndex>=mActivity.phoneAlbums.size())
            return "";

        PhoneAlbum phoneAlbum = mActivity.phoneAlbums.get(groupIndex);

        if (phoneAlbum.getAlbumPhotos().size()!=phoneAlbum.getPhotoCount()) {
            phoneAlbum.getAlbumPhotos().clear();;
            phoneAlbum = DeviceImageManager.getPhotosInfo(mActivity.getApplicationContext(), phoneAlbum);
        }


        if (photoIndex>=phoneAlbum.getAlbumPhotos().size())
            return "";

        PhonePhoto phonePhoto = phoneAlbum.getAlbumPhotos().get(photoIndex);
        return phonePhoto.getPhotoUri();
    }

    static public int getPhotoOrientationFromUrl(String imageUrl) {
        // find photo
        PhoneAlbum phoneAlbum = mActivity.phoneAlbums.firstElement();

        PhonePhoto phonePhoto= null;
        for (PhonePhoto photo : phoneAlbum.getAlbumPhotos()) {
            if (photo.getPhotoUri().compareTo(imageUrl)==0) {
                phonePhoto = photo;
                break;
            }
        }

        if (phonePhoto==null) {
            return 0;
        }

        return phonePhoto.getOrientation();
    }

    static public byte[] getPhotoImage(String imageUrl) {
        // find photo
        PhoneAlbum phoneAlbum = mActivity.phoneAlbums.firstElement();

        PhonePhoto phonePhoto= null;
        for (PhonePhoto photo : phoneAlbum.getAlbumPhotos()) {
            if (photo.getPhotoUri().compareTo(imageUrl)==0) {
                phonePhoto = photo;
                break;
            }
        }

        if (phonePhoto==null) {
            return null;
        }

        int orientation = phonePhoto.getOrientation();
        int sideLength = MAX_SIDE_LENGTH;
        Bitmap bitmap = DeviceImageManager.loadBitmapResize(imageUrl, orientation, sideLength);

        if (bitmap == null) {
            return null;
        }
//        Log.i("JAVA", "[[[[[ get bitmap width : " + Integer.toString(bitmap.getWidth()) + ", height : " + Integer.toString(bitmap.getHeight()) + ", maxLength : " + Integer.toString(sideLength));

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        boolean compressResult = false;
        byte[] byteBuffer = null;
        try {
            if (compressResult = bitmap.compress(Bitmap.CompressFormat.JPEG, 100, baos)) {
                byteBuffer = baos.toByteArray();
            }
        } catch (IllegalStateException e) {
            // Does nothing
        } finally {
            baos = null;
        }

        if (!compressResult) {
            return null;
        }

        if (byteBuffer != null && byteBuffer.length > 0) {
            return byteBuffer;
        }

        return null;
    }


    static public byte[] getPhotoThumbnail(String imageUrl) {

        // find photo
        PhoneAlbum phoneAlbum = mActivity.phoneAlbums.firstElement();

        PhonePhoto phonePhoto= null;
        for (PhonePhoto photo : phoneAlbum.getAlbumPhotos()) {
            if (photo.getPhotoUri().compareTo(imageUrl)==0) {
                phonePhoto = photo;
                break;
            }
        }

        if (phonePhoto==null) {
            return null;
        }

        String thumbPath = DeviceImageManager.getThumbnailPath(mActivity.getApplicationContext(), phonePhoto.getId());
        Bitmap bitmap = null;
        int orientation = phonePhoto.getOrientation();
        int sideLength = DEFAULT_SIDE_LENGTH;
        if (thumbPath != null) {
            bitmap = DeviceImageManager.extractThumbnailFromFile(thumbPath, orientation, sideLength, sideLength);
        }

        if (bitmap == null) {
            bitmap = DeviceImageManager.extractThumbnailFromFile(phonePhoto.getPhotoUri(), orientation, sideLength, sideLength);
        }

        if (bitmap == null) {
            return null;
        }

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        boolean compressResult = false;
        byte[] byteBuffer = null;
        try {
            if (compressResult = bitmap.compress(Bitmap.CompressFormat.JPEG, 80, baos)) {
                byteBuffer = baos.toByteArray();
            }
        } catch (IllegalStateException e) {
            // Does nothing
        } finally {
            baos = null;
        }

        if (!compressResult) {
            return null;
        }

        if (byteBuffer != null && byteBuffer.length > 0) {
            return byteBuffer;
        }

        return null;
    }





/*
    // get photo URLs
    static public String[] getPhotoUrls(int groupIndex) {
        ArrayList<String> photoUrls = new ArrayList<String>();
        if (groupIndex<mActivity.phoneAlbums.size()) {
            PhoneAlbum phoneAlbum = mActivity.phoneAlbums.get(groupIndex);
            for (PhonePhoto phonePhoto : phoneAlbum.getAlbumPhotos()) {
                photoUrls.add(phonePhoto.getPhotoUri());
            }

        }

        String[] retArray = new String[photoUrls.size()];
        retArray =photoUrls.toArray(retArray);

        return retArray;
    }




    // get group list JNI
    static public String[] getGroupList() {

        Log.d("album list", "album count : " + mActivity.phoneAlbums.size());

        for (PhoneAlbum album : mActivity.phoneAlbums) {
            Log.d("album list", "album name : " + album.getName());
        }

        Uri uri;
        Cursor cursor;
        int column_index_data, column_index_folder_name;
        ArrayList<String> listOfAllImages = new ArrayList<String>();

        String absolutePathOfImage = null;
        uri = android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI;

        String[] projection = { MediaStore.MediaColumns.DATA,
                MediaStore.Images.Media.BUCKET_DISPLAY_NAME };

        cursor = mActivity.getContentResolver().query(uri, projection, null, null, null);

        column_index_data = cursor.getColumnIndexOrThrow(MediaStore.MediaColumns.DATA);
        column_index_folder_name = cursor
                .getColumnIndexOrThrow(MediaStore.Images.Media.BUCKET_DISPLAY_NAME);
        while (cursor.moveToNext()) {
            absolutePathOfImage = cursor.getString(column_index_data);

            listOfAllImages.add(absolutePathOfImage);
        }


        String[] retArray = new String[listOfAllImages.size()];
        retArray =listOfAllImages.toArray(retArray);
        return retArray;
    }
*/

//    public String getAlbumName()


    public void finishEditImage(String filePath) {
//        Log.d("EDIT_IMAGE_FINISH", "[[[[[[[[[[[[[[[[[[[ save file Path : " + filePath);
        Intent intent = new Intent();

        intent.putExtra("EDIT_IMAGE", filePath);
        intent.putExtra("JUST_CLOSE", false);

        setResult(RESULT_OK, intent);


        finish();
    }

    public static void editImageFinish(String filePath) {
        if(mActivity == null)
            return;
        mActivity.finishEditImage(filePath);
    }

    @Override
    public void onDestroy() {
        Cocos2dxHelper.terminateProcess();
//        SMImageEditorActivity.nativeFinish();

//        System.gc();
        super.onDestroy();

//        unregisterManagers();
    }


    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.d("TEST", "TESTLOG");
    }

    public static void testCallJNI() {
        if(mActivity == null)
            return;

        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {

                Intent testIntent = new Intent();
                testIntent.putExtra("TEST_STRING", "TEST_STRING_");

                if (testIntent.resolveActivity(mActivity.getPackageManager()) != null)
                    mActivity.startActivityForResult(testIntent, JNI_TEST_STRING);
            }
        });
    }
    /*
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.setEnableVirtualButton(false);
        super.onCreate(savedInstanceState);
        // Workaround in https://stackoverflow.com/questions/16283079/re-launch-of-activity-on-home-button-but-only-the-first-time/16447508
        if (!isTaskRoot()) {
            // Android launched another instance of the root activity into an existing task
            //  so just quietly finish and go away, dropping the user back into the activity
            //  at the top of the stack (ie: the last state of this task)
            // Don't need to finish it again since it's finished in super.onCreate .
            return;
        }
        // DO OTHER INITIALIZATION BELOW
        
    }
*/
}
