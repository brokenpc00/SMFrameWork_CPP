package com.brokenpc.SMCocosLib;

/**
 * Created by stevemac on 2017. 10. 17..
 */

import android.graphics.Bitmap;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;


public class CameraPreviewBuffer {
    public static final int NUM_PREVIEW_BUFFER = 2;
    private byte[][] mDataBuffer;

    private int mWidth;
    private int mHeight;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private int mOrientation;
    private boolean mFlipHorizontal;
    private boolean mFlipVertical;
    private float mScale = 1f;

    private ByteBuffer mYBuffer;
    private ByteBuffer mUVBuffer;

    // 이거 없어도 됨
//    private FloatBuffer vertices;
//    private FloatBuffer texcoord;

    private boolean mDataAvailable;
    private boolean mSpriteReady;

    public static CameraPreviewBuffer createPreviewBuffer(int width, int height, int previewWidth, int previewHeight, int orientation, boolean hflip, boolean vflip) {
        return new CameraPreviewBuffer(width, height, previewWidth, previewHeight, orientation, hflip, vflip);
    }

    private CameraPreviewBuffer(int width, int height, int pw, int ph, int orientation, boolean hflip, boolean vflip) {
        mWidth = width;
        mHeight = height;
        mPreviewWidth = pw;
        mPreviewHeight = ph;
        mDataAvailable = false;
        mSpriteReady = false;
        mOrientation = orientation;
        mFlipHorizontal = hflip;
        mFlipVertical = vflip;
        final float sw = mWidth;
        if (mOrientation == 90 || mOrientation == 270) {
            mScale = sw/ph;
        } else {
            mScale = sw/pw;
        }
    }

    public boolean isSpriteReady() {
        return mSpriteReady;
    }

    public int getPreviewWidth() {
        return mPreviewWidth;
    }

    public int getPreviewHeight() {
        return mPreviewHeight;
    }

    public byte[] getDataBuffer(int index) {
        if (mDataBuffer == null) {
            mDataBuffer = new byte[NUM_PREVIEW_BUFFER][];
        }
        if (index >= 0 && index < NUM_PREVIEW_BUFFER) {
            if (mDataBuffer[index] == null) {
                mDataBuffer[index] = new byte[mPreviewWidth * mPreviewHeight * 3];
            }
            return mDataBuffer[index];
        }
        return null;
    }

    // 이게 쓰일지는 모르겠음
    public synchronized void putData(byte[] data) {
        // data는 mDataBuffer 와 같음
        if (mYBuffer == null) {
            mYBuffer = ByteBuffer.allocateDirect(mPreviewWidth * mPreviewHeight).order(ByteOrder.nativeOrder());
        }
        mYBuffer.put(data, 0, mPreviewWidth * mPreviewHeight);
        mYBuffer.position(0);

        if (mUVBuffer == null) {
            mUVBuffer = ByteBuffer.allocateDirect(mPreviewWidth * mPreviewHeight / 2).order(ByteOrder.nativeOrder());
        }
        mUVBuffer.put(data, mPreviewWidth * mPreviewHeight, mPreviewWidth * mPreviewHeight / 2);
        mUVBuffer.position(0);

        mDataAvailable = true;
    }

    public void updatePreviewBuffer() {
        updatePreviewBuffer(0);
    }

    // 이것도 쓰일지는 모르겠음
    public synchronized void updatePreviewBuffer(int bilateralSteps) {
        if (!mDataAvailable)
            return;

        mDataAvailable = false;
        mSpriteReady = true;
    }

    public synchronized Bitmap getPreviewBitmap() {
        // 현재 화면(preview)를 bitmap으로... 근데 안쓰임
        return null;
    }

    // 이거는 외부에서 쓰임
    public synchronized void releaseBuffer() {
        mDataBuffer = null;
        mYBuffer = null;
        mUVBuffer = null;
        mDataAvailable = false;
    }

    public void removeBuffer() {
        if (mDataBuffer != null) {
            for (int i = 0; i < NUM_PREVIEW_BUFFER; i++) {
                mDataBuffer[i] = null;
            }
            mDataBuffer = null;
        }
    }

}
