package com.brokenpc.SMCocosLib;

/**
 * Created by stevemac on 2017. 10. 17..
 */

import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.net.Uri;
import android.provider.MediaStore;
import android.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Vector;



/**
 * Created by KimJaePhil_iMac on 2017. 7. 7..
 */

public class DeviceImageManager {
    public static void getPhoneAlbumInfo(Context context, OnPhoneImagesObtained listener) {
        Vector<PhoneAlbum> items = new Vector<>();
        final String[] PROJECTION_BUCKET = {
                MediaStore.Images.ImageColumns.BUCKET_ID,
                MediaStore.Images.ImageColumns.BUCKET_DISPLAY_NAME,
                MediaStore.Images.ImageColumns.DATA,
                MediaStore.Images.ImageColumns._ID,
                MediaStore.Images.ImageColumns.ORIENTATION
        };

        final String BUCKET_GROUP_BY = "1) GROUP BY 1,(2";
        final String BUCKET_ORDER_BY = "MAX("+MediaStore.Images.ImageColumns.DATE_ADDED+") DESC";

        Cursor cursor = context.getContentResolver().query(
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                PROJECTION_BUCKET, BUCKET_GROUP_BY, null, BUCKET_ORDER_BY);

        if (cursor != null) {
            if (cursor.moveToFirst()) {
                long bucketId;
                int id;
                String bucketName;
                String data;
                String imageId;
                int orientation;

                int idColumn = cursor.getColumnIndex(MediaStore.Images.Media._ID);
                int bucketIdColumn = cursor.getColumnIndex(MediaStore.Images.Media.BUCKET_ID);
                int bucketColumn = cursor.getColumnIndex(MediaStore.Images.Media.BUCKET_DISPLAY_NAME);
                int dataColumn = cursor.getColumnIndex(MediaStore.Images.Media.DATA);
                int orientationColumn = cursor.getColumnIndex(MediaStore.Images.Media.ORIENTATION);
                int nIndex = 0;
                do {
                    id = cursor.getInt(idColumn);
                    bucketId = cursor.getLong(bucketIdColumn);
                    bucketName = cursor.getString(bucketColumn);
                    data = cursor.getString(dataColumn);
                    orientation = cursor.getInt(orientationColumn);

                    PhoneAlbum item = new PhoneAlbum();
                    item.setId(id);
                    item.setBucketId(bucketId);
                    item.setCoverUri(data);
                    item.setName(bucketName);
                    item.setAlbumIndex(nIndex);
                    items.add(item);
                    nIndex++;

                } while (cursor.moveToNext());
            }


            int totalCount = 0;
            int numItems = items.size();
            for (int i = numItems-1; i >= 0; i--) {
                PhoneAlbum item = items.get(i);
                int count = countInBucket(context, item.getBucketId());
                if (count > 0) {
                    item.setPhotoCount(count);
                    totalCount += count;
                } else {
                    items.remove(i);
                }
            }

            if (items.size()<=0) {
                listener.onError();
                return;
            }

            PhoneAlbum firstAlbum = items.firstElement();
            String totalCoverUrl = firstAlbum.getCoverUri();

            // All photos 추가
            PhoneAlbum allPhotoItem = new PhoneAlbum();
            allPhotoItem.setId(0);
            allPhotoItem.setBucketId(0);
            allPhotoItem.setCoverUri(totalCoverUrl);
            allPhotoItem.setName("ALL PHOTOS");
            allPhotoItem.setAlbumIndex(-99);
            allPhotoItem.setPhotoCount(totalCount);
            items.add(0, allPhotoItem);
            cursor.close();
        }

        if (items.size()>0) {
            listener.onLoadAlbumComplete(items);
        } else {
            listener.onError();;
        }
    }

    private static int countInBucket(Context context, long bucketId) {
        final String[] PROJECTION_COUNT = {
                "count("+MediaStore.Images.Media._ID+")"
        };
        final String WHERE_CLAUSE = MediaStore.Images.Media.BUCKET_ID + " = ?";
        final String[] WHERE_ARG = new String[] { String.valueOf(bucketId) };

        Cursor cursor = context.getContentResolver().query(
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                PROJECTION_COUNT,
                WHERE_CLAUSE, WHERE_ARG, null);


        int countInBucket = 0;
        if (cursor != null) {
            if (cursor.moveToFirst()) {
                countInBucket = cursor.getInt(0);
            }
            cursor.close();
        }

        return countInBucket;
    }

    public static PhoneAlbum getPhotosInfo(Context context, PhoneAlbum album) {

        final String[] PROJECTION_BUCKET = {
                MediaStore.Images.Media.DATA,
                MediaStore.Images.Media._ID,
                MediaStore.Images.Media.ORIENTATION,
                MediaStore.Images.Media.DATE_ADDED,
        };

        final String ORDER_BY = MediaStore.Images.Media.DATE_ADDED;
        final String WHERE_CLAUSE;
        final String[] WHERE_ARG;

        long bucketId = album.getBucketId();
        if (bucketId != 0 && album.getAlbumIndex() >= 0) {
            WHERE_CLAUSE = MediaStore.Images.Media.BUCKET_ID + " = ?";
            WHERE_ARG = new String[] { String.valueOf(bucketId) };
        } else {
            WHERE_CLAUSE = null;
            WHERE_ARG = null;
        }

        Cursor cur = context.getContentResolver().query(
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                PROJECTION_BUCKET,
                WHERE_CLAUSE, WHERE_ARG,
                ORDER_BY + " DESC");


        if (cur!=null && cur.getCount()>0) {
            if (cur.moveToFirst()) {
                String data;
                int id;
                int orientation;
                int imageUriColumn = cur.getColumnIndex(MediaStore.Images.Media.DATA);
                int imageIdColumn = cur.getColumnIndex(MediaStore.Images.Media._ID);
                int orientationIdColumn = cur.getColumnIndex(MediaStore.Images.Media.ORIENTATION);

                do {
                    // get the field vlaues
                    id = cur.getInt(imageIdColumn);
                    orientation = cur.getInt(orientationIdColumn);
                    data = cur.getString(imageUriColumn);

                    PhonePhoto phonePhoto = new PhonePhoto();
                    phonePhoto.setAlbumName(album.getName());
                    phonePhoto.setPhotoUri(data);
                    phonePhoto.setId(id);
                    phonePhoto.setOrientation(orientation);;
                    phonePhoto.setPhotoIndex(album.getAlbumPhotos().size());
                    album.getAlbumPhotos().add(phonePhoto);

                } while (cur.moveToNext());
            }
        }

        return album;
    }

    public static String getThumbnailPath(Context context, long imageId) {
        Cursor cursor = null;
        String filePath = null;

        try {
            cursor = MediaStore.Images.Thumbnails.queryMiniThumbnail(context.getContentResolver(), imageId,
                    MediaStore.Images.Thumbnails.MINI_KIND, null);

            if (cursor != null && cursor.moveToFirst()) {
                filePath = cursor.getString(cursor.getColumnIndex(MediaStore.Images.Thumbnails.DATA));
            }
        } catch (Exception e) {
            // Does nothing
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }

        return filePath;
    }


    public static Bitmap loadBitmap(Context context, final String path, final int degrees, final int reqWidth, final int reqHeight) {
        return new DeviceImageManager().internalLoadBitmapExactly(context, path, degrees, reqWidth, reqHeight);
    }

    public static Bitmap resizeBitmap(final Bitmap src, final int degrees, final int reqWidth, final int reqHeight) {
        return new DeviceImageManager().internalResizeBitmap(src, degrees, reqWidth, reqHeight);
    }

    public static Bitmap loadBitmapRoughly(Context context, final String path, final int degrees, final int reqWidth, final int reqHeight) {
        return new DeviceImageManager().internalLoadRoughly(context, path, degrees, reqWidth, reqHeight);
    }

    private Bitmap internalLoadRoughly(Context context, final String path, final int degrees, final int reqWidth, final int reqHeight) {
        if (path == null)
            return null;

        Bitmap bitmap = null;
        InputStream is = null;

        int outWidth = 0;
        int outHeight = 0;
        Uri uri = Uri.fromFile(new File(path));

        if (uri == null)
            return null;

        BitmapFactory.Options options = new BitmapFactory.Options();

        try {
            is = context.getContentResolver().openInputStream(uri);
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeStream(is, null, options);
        } catch (FileNotFoundException e) {
            return null;
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    // Does nothing
                }
                is = null;
            }
        }

        if (degrees == 90 || degrees == 270) {
            outWidth = options.outHeight;
            outHeight = options.outWidth;
        } else {
            outWidth = options.outWidth;
            outHeight = options.outHeight;
        }

        // 비트맵 로드
        options.inSampleSize = calculateInSampleSize(outWidth, outHeight, reqWidth, reqHeight);
        options.inJustDecodeBounds = false;
        options.inPreferredConfig = Bitmap.Config.RGB_565;

        int retry = 0;
        while (bitmap == null && retry < 5) {
            try {
                is = context.getContentResolver().openInputStream(uri);
                bitmap = BitmapFactory.decodeStream(is, null, options);
                if (bitmap != null) {
                    break;
                }
            } catch (OutOfMemoryError e) {
                System.gc();
            } catch (FileNotFoundException e) {
                break;
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {
                        // Does nothing
                    }
                    is = null;
                }
            }
            retry += 1;
            options.inSampleSize += 1;
        }

        if (bitmap != null && degrees != 0) {

            Matrix matrix = new Matrix();
            float width, height;
            if (degrees == 90 || degrees == 270) {
                width = bitmap.getHeight();
                height = bitmap.getWidth();
            } else {
                width = bitmap.getWidth();
                height = bitmap.getHeight();
            }

            Bitmap rotated = null;
            retry = 0;
            float scale = 1;
            while (rotated == null && retry < 3) {
                try {
                    rotated = Bitmap.createBitmap((int)width, (int)height, Bitmap.Config.RGB_565);
                    if (rotated != null) {
                        matrix.reset();
                        matrix.postTranslate(-bitmap.getWidth()/2f, -bitmap.getHeight()/2f);
                        matrix.postRotate(degrees);
                        matrix.postScale(scale, scale);
                        matrix.postTranslate(width/2f, height/2f);

                        Canvas canvas = new Canvas(rotated);
                        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG);
                        paint.setDither(true);
                        canvas.drawBitmap(bitmap, matrix, paint);
                        break;
                    }
                } catch (OutOfMemoryError e) {
                    System.gc();
                }

                retry += 1;
                width /= 2f;
                height/= 2f;
                scale /= 2f;
            }

            if (rotated != null) {
                if (bitmap != null && !bitmap.isRecycled()) {
                    bitmap.recycle();
                    bitmap = null;
                }
                bitmap = rotated;
            }
        }

        return bitmap;
    }

    private Bitmap internalResizeBitmap(Bitmap bitmap, final int degrees, final int reqWidth, final int reqHeight) {
        if (bitmap == null)
            return null;

        if (bitmap != null &&
                (degrees != 0 || bitmap.getWidth()*bitmap.getHeight() > reqWidth*reqHeight)) {

            Matrix matrix = new Matrix();
            float width, height;
            if (degrees == 90 || degrees == 270) {
                width = bitmap.getHeight();
                height = bitmap.getWidth();
            } else {
                width = bitmap.getWidth();
                height = bitmap.getHeight();
            }

            Bitmap rotated = null;
            float scale = Math.max((float)reqWidth/width, (float)reqHeight/height);
            int retry = 0;
            width = reqWidth;
            height = reqHeight;
            while (rotated == null && retry < 3) {
                try {
                    rotated = Bitmap.createBitmap((int)width, (int)height, Bitmap.Config.ARGB_8888);
                    if (rotated != null) {
                        matrix.reset();
                        matrix.postTranslate(-bitmap.getWidth()/2f, -bitmap.getHeight()/2f);
                        matrix.postRotate(degrees);
                        matrix.postScale(scale, scale);
                        matrix.postTranslate(width/2f, height/2f);

                        Canvas canvas = new Canvas(rotated);
                        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG);
                        paint.setDither(true);
                        canvas.drawBitmap(bitmap, matrix, paint);
                        break;
                    }
                } catch (OutOfMemoryError e) {
                    System.gc();
                }

                retry += 1;
                width /= 2f;
                height/= 2f;
                scale /= 2f;
            }

            if (rotated != null && rotated != bitmap) {
                if (bitmap != null && !bitmap.isRecycled()) {
                    bitmap.recycle();
                    bitmap = null;
                }
                bitmap = rotated;
                rotated = null;
            }
        }

        return bitmap;
    }

    private static int calculateInSampleSize(int width, int height, int reqWidth, int reqHeight) {

        int inSampleSize = 1;

        if (width > reqWidth || height > reqHeight) {
            if (width > height) {
                inSampleSize = Math.round((float) height / (float) reqHeight);
            } else {
                inSampleSize = Math.round((float) width / (float) reqWidth);
            }

            final float totalPixels = width * height;
            final float totalReqPixelsCap = reqWidth * reqHeight * 2;

            while (totalPixels / (inSampleSize * inSampleSize) > totalReqPixelsCap) {
                inSampleSize++;
            }
        }
        return inSampleSize;
    }

    public static Bitmap loadBitmapResize(final String pathName, final int degrees, final int maxSize) {
        if (pathName == null)
            return null;

        Bitmap bitmap = null;

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(pathName, options);

        final int outWidth, outHeight;
        if (degrees == 90 || degrees == 270) {
            outWidth = options.outHeight;
            outHeight = options.outWidth;
        } else {
            outWidth = options.outWidth;
            outHeight = options.outHeight;
        }


        boolean bNeedResize = false;
        float ratio = 1.0f;

        if (outWidth>maxSize || outHeight>maxSize) {    // 기준 크기보다 크냐
            float widthRatio = (float)maxSize/outWidth;
            float heightRatio = (float)maxSize/outHeight;
            ratio = Math.min(widthRatio, heightRatio);
        }
        int newWidth = (int)((float)outWidth * ratio);
        int newHeight = (int)((float)outHeight * ratio);

        Log.i("IMAGE MANGER", "[[[[[ out width : " + Integer.toString(outWidth) + ", out height : " + Integer.toString(outHeight) + ", new width : " + Integer.toString(newWidth) + ", new height : " + Integer.toString(newHeight));

        // 비트맵 로드
        options.inJustDecodeBounds = false;
        options.inSampleSize = Math.max(1, Math.max(maxSize/outWidth, maxSize/outHeight));
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;

        int retry = 0;
        while (bitmap == null && retry < 5) {
            try {
                bitmap = BitmapFactory.decodeFile(pathName, options);
                if (bitmap != null) {
                    break;
                }
            } catch (OutOfMemoryError e) {
                System.gc();
            }
            retry += 1;
            options.inSampleSize += 1;
        }

        Log.i("IMAGE MANAGER", "[[[[[ create bitmap ~~~~~ : orgin(" + Integer.toString(outWidth) + ", " + Integer.toString(outHeight) + "), new(" + Integer.toString(newWidth) + ", " + Integer.toString(newHeight) + ")");

        //return createScaledBitmap(bitmap, degrees, newWidth, newHeight, false);

//        if (bitmap!=null) {
//            Matrix matrix = new Matrix();
//            // RESIZE THE BIT MAP
//            float widthScale = ((float) newWidth) / outWidth;
//            float heightScale = ((float) newHeight) / outHeight;
//            matrix.postScale(widthScale, heightScale);
//            matrix.setRotate(degrees);
//            Log.i("IMAGE MANGER", "[[[[[ resize image widthScale : " + Float.toString(widthScale) + ", heightScale : " + Float.toString(heightScale) + ", degrees : " + Integer.toString(degrees));
//            // "RECREATE" THE NEW BITMAP
//            Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, outWidth, outHeight, matrix, false);
//          //  bitmap.recycle();
//
//
//            return resizedBitmap;
//        }
//
//        return bitmap;


        if (bitmap != null &&
                (degrees != 0 || bitmap.getWidth()*bitmap.getHeight() > newWidth*newHeight)) {

            Matrix matrix = new Matrix();
            float width, height;
            if (degrees == 90 || degrees == 270) {
                width = bitmap.getHeight();
                height = bitmap.getWidth();
            } else {
                width = bitmap.getWidth();
                height = bitmap.getHeight();
            }

            Bitmap rotated = null;
            float scaleX = (float)newWidth/width;
            float scaleY = (float)newHeight/height;
            float scale = Math.max(scaleX, scaleY);
            retry = 0;
            width = newWidth;
            height = newHeight;
            while (rotated == null && retry < 3) {
                try {
                    rotated = Bitmap.createBitmap((int)width, (int)height, Bitmap.Config.ARGB_8888);
                    if (rotated != null) {
                        matrix.reset();
                        matrix.postTranslate(-bitmap.getWidth()/2f, -bitmap.getHeight()/2f);
                        matrix.postRotate(degrees);
                        matrix.postScale(scale, scale);
                        matrix.postTranslate(width/2f, height/2f);

                        Canvas canvas = new Canvas(rotated);
                        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG);
                        paint.setDither(true);
                        canvas.drawBitmap(bitmap, matrix, paint);
                        break;
                    }
                } catch (OutOfMemoryError e) {
                    System.gc();
                }

                retry += 1;
                width /= 2f;
                height/= 2f;
                scaleX /= 2f;
                scaleY /= 2f;
            }

            if (rotated != null) {
                if (bitmap != null && !bitmap.isRecycled()) {
                    bitmap.recycle();
                    bitmap = null;
                }
                bitmap = rotated;
            }
        }
        Log.i("IMAGE MANAGER", "[[[[[ load image from file on JNI complete!!!!");
        return bitmap;
    }

    private static volatile Matrix sScaleMatrix;

    public static Bitmap createScaledBitmap(Bitmap src, final int degrees, int dstWidth, int dstHeight,
                                            boolean filter) {
        Matrix m;
        synchronized (Bitmap.class) {
            // small pool of just 1 matrix
            m = sScaleMatrix;
            sScaleMatrix = null;
        }
        if (m == null) {
            m = new Matrix();
        }
        final int width = src.getWidth();
        final int height = src.getHeight();
        final float sx = dstWidth  / (float)width;
        final float sy = dstHeight / (float)height;
        m.setScale(sx, sy);
        m.setRotate(degrees);

        Log.i("IMAGE MANAGER", "[[[[[ create resize bitmap 1 ~~~~~");
        Bitmap b = Bitmap.createBitmap(src, 0, 0, width, height, m, filter);
        synchronized (Bitmap.class) {
            // do we need to check for null? why not just assign everytime?
            if (sScaleMatrix == null) {
                sScaleMatrix = m;
            }
        }
        Log.i("IMAGE MANAGER", "[[[[[ create resize bitmap 2 ~~~~~");
        return b;
    }

    public static Bitmap loadBitmapMaxSize(final String pathName, final int degrees, final int maxSize) {
        if (pathName == null)
            return null;

        Bitmap bitmap = null;

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(pathName, options);

        final int outWidth, outHeight;
        if (degrees == 90 || degrees == 270) {
            outWidth = options.outHeight;
            outHeight = options.outWidth;
        } else {
            outWidth = options.outWidth;
            outHeight = options.outHeight;
        }

        int scaleSize;
        float widthRatio = (float)outWidth/(float)maxSize;
        float heightRatio = (float)outHeight/(float)maxSize;
        Log.d("BITMAPMAXSIZE", "[[[[[[[[[[ widthRation : " + Float.toString(widthRatio) + ", heightRatio : " + Float.toString(heightRatio) + ", MAXSIZE : " + Integer.toString(maxSize));
        float ratio = Math.min(widthRatio, heightRatio);
        if (ratio>1.0f) {
            ratio = 1.0f;
        }

        int newWidth = (int)((float)outWidth*ratio);
        int newHeight = (int)((float)outHeight*ratio);

        // 비트맵 로드
        options.inJustDecodeBounds = false;
        options.inSampleSize = Math.max(1, Math.max(outWidth/maxSize, outHeight/maxSize));
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;

        int retry = 0;
        while (bitmap == null && retry < 5) {
            try {
                bitmap = BitmapFactory.decodeFile(pathName, options);
                if (bitmap != null) {
                    break;
                }
            } catch (OutOfMemoryError e) {
                System.gc();
            }
            retry += 1;
            options.inSampleSize += 1;
        }

        if (bitmap != null &&
                (degrees != 0 || bitmap.getWidth()*bitmap.getHeight() > newWidth*newHeight)) {

            Matrix matrix = new Matrix();
            float width, height;
            if (degrees == 90 || degrees == 270) {
                width = bitmap.getHeight();
                height = bitmap.getWidth();
            } else {
                width = bitmap.getWidth();
                height = bitmap.getHeight();
            }

            Bitmap rotated = null;
            float scaleX = (float)newWidth/width;
            float scaleY = (float)newHeight/height;
            float scale = Math.max(scaleX, scaleY);
            retry = 0;
            width = newWidth;
            height = newHeight;
            while (rotated == null && retry < 3) {
                try {
                    rotated = Bitmap.createBitmap((int)width, (int)height, Bitmap.Config.ARGB_8888);
                    if (rotated != null) {
                        matrix.reset();
                        matrix.postTranslate(-bitmap.getWidth()/2f, -bitmap.getHeight()/2f);
                        matrix.postRotate(degrees);
                        matrix.postScale(scale, scale);
                        matrix.postTranslate(width/2f, height/2f);

                        Canvas canvas = new Canvas(rotated);
                        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG);
                        paint.setDither(true);
                        canvas.drawBitmap(bitmap, matrix, paint);
                        break;
                    }
                } catch (OutOfMemoryError e) {
                    System.gc();
                }

                retry += 1;
                width /= 2f;
                height/= 2f;
                scaleX /= 2f;
                scaleY /= 2f;
            }

            if (rotated != null) {
                if (bitmap != null && !bitmap.isRecycled()) {
                    bitmap.recycle();
                    bitmap = null;
                }
                bitmap = rotated;
            }
        }

        return bitmap;
    }

    private Bitmap internalLoadBitmapExactly(Context context, final String pathName, final int degrees, final int reqWidth, final int reqHeight) {
        if (pathName == null)
            return null;

        Bitmap bitmap = null;

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(pathName, options);

        final int outWidth, outHeight;
        if (degrees == 90 || degrees == 270) {
            outWidth = options.outHeight;
            outHeight = options.outWidth;
        } else {
            outWidth = options.outWidth;
            outHeight = options.outHeight;
        }

        // 비트맵 로드
        options.inJustDecodeBounds = false;
        options.inSampleSize = Math.max(1, Math.max(outWidth/reqWidth, outHeight/reqHeight));
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;

        int retry = 0;
        while (bitmap == null && retry < 5) {
            try {
                bitmap = BitmapFactory.decodeFile(pathName, options);
                if (bitmap != null) {
                    break;
                }
            } catch (OutOfMemoryError e) {
                System.gc();
            }
            retry += 1;
            options.inSampleSize += 1;
        }

        if (bitmap != null &&
                (degrees != 0 || bitmap.getWidth()*bitmap.getHeight() > reqWidth*reqHeight)) {

            Matrix matrix = new Matrix();
            float width, height;
            if (degrees == 90 || degrees == 270) {
                width = bitmap.getHeight();
                height = bitmap.getWidth();
            } else {
                width = bitmap.getWidth();
                height = bitmap.getHeight();
            }

            Bitmap rotated = null;
            float scaleX = (float)reqWidth/width;
            float scaleY = (float)reqHeight/height;
            float scale = Math.max(scaleX, scaleY);
            retry = 0;
            width = reqWidth;
            height = reqHeight;
            while (rotated == null && retry < 3) {
                try {
                    rotated = Bitmap.createBitmap((int)width, (int)height, Bitmap.Config.ARGB_8888);
                    if (rotated != null) {
                        matrix.reset();
                        matrix.postTranslate(-bitmap.getWidth()/2f, -bitmap.getHeight()/2f);
                        matrix.postRotate(degrees);
                        matrix.postScale(scale, scale);
                        matrix.postTranslate(width/2f, height/2f);

                        Canvas canvas = new Canvas(rotated);
                        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG);
                        paint.setDither(true);
                        canvas.drawBitmap(bitmap, matrix, paint);
                        break;
                    }
                } catch (OutOfMemoryError e) {
                    System.gc();
                }

                retry += 1;
                width /= 2f;
                height/= 2f;
                scaleX /= 2f;
                scaleY /= 2f;
            }

            if (rotated != null) {
                if (bitmap != null && !bitmap.isRecycled()) {
                    bitmap.recycle();
                    bitmap = null;
                }
                bitmap = rotated;
            }
        }

        return bitmap;
    }

    public static Bitmap extractThumbnailFromFile(final String filePath, final int degrees, final int reqWidth, final int reqHeight) {
        if (filePath == null)
            return null;

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(filePath, options);

        final int outWidth, outHeight;
        if (degrees == 90 || degrees == 270) {
            outWidth = options.outHeight;
            outHeight = options.outWidth;
        } else {
            outWidth = options.outWidth;
            outHeight = options.outHeight;
        }

        if (outWidth <= 0 || outHeight <= 0)
            return null;

        // 비트맵 로드
        options.inJustDecodeBounds = false;
        options.inSampleSize = Math.max(1, Math.max(outWidth/reqWidth, outHeight/reqHeight));
        options.inPreferredConfig = Bitmap.Config.RGB_565;

        int retry = 0;
        Bitmap bitmap = null;

        do {
            try {
                bitmap = BitmapFactory.decodeFile(filePath, options);
                if (bitmap != null) {
                    break;
                }
            } catch (Throwable t) {
                System.gc();
            }
            retry += 1;
            options.inSampleSize += 1;
        } while (retry < 5);



        // 읽기 실패
        if (bitmap == null) {
            Log.i("IMAGE MANAGER", "[[[[[ DECODE IMAGE IS NULL!!!");
            System.gc();
            return null;
        }

        // 이미지 줄일 필요 없음
        if (degrees == 0 && bitmap.getWidth() == reqWidth && bitmap.getHeight() == reqHeight)
            return bitmap;

        float width, height;
        if (degrees == 90 || degrees == 270) {
            width = bitmap.getHeight();
            height = bitmap.getWidth();
        } else {
            width = bitmap.getWidth();
            height = bitmap.getHeight();
        }

        float scaleX = reqWidth/width;
        float scaleY = reqHeight/height;
        float scale = Math.max(scaleX, scaleY);
        retry = 0;
        width = reqWidth;
        height = reqHeight;

        Bitmap newBitmap = null;
        do {
            try {
                newBitmap = Bitmap.createBitmap((int)width, (int)height, Bitmap.Config.RGB_565);
                if (newBitmap != null) {
                    Matrix matrix = new Matrix();
                    matrix.postTranslate(-bitmap.getWidth()/2f, -bitmap.getHeight()/2f);
                    matrix.postRotate(degrees);
                    matrix.postScale(scale, scale);
                    matrix.postTranslate(width/2f, height/2f);
                    new Canvas(newBitmap).drawBitmap(bitmap, matrix, new Paint(Paint.FILTER_BITMAP_FLAG));
                    break;
                }
            } catch (Throwable t) {
                System.gc();
            }

            retry++;
            width /= 2f;
            height /= 2f;
            scaleX /= 2f;
            scaleY /= 2f;
        } while (retry < 3);

        if (newBitmap != null) {
            if (!bitmap.isRecycled()) {
                bitmap.recycle();
                bitmap = null;
            }
            bitmap = newBitmap;
        }

        return bitmap;
    }

    /*

    private static final int SHORT_SIDE_LENGTH = 960;
    private static final int LONG_SIDE_LENGTH = 1280;

    public static Bitmap createPictureBitmap(byte[] data, int degrees, boolean flip) {

        Bitmap bitmap = null;

        ExifData exifData = new ExifData();
        exifData.readExif(data);

        int orientation = 0;
        switch (exifData.Orientation) {
            case ExifInterface.ORIENTATION_ROTATE_90:
                orientation = 90;
                break;
            case ExifInterface.ORIENTATION_ROTATE_180:
                orientation = 90;
                break;
            case ExifInterface.ORIENTATION_ROTATE_270:
                orientation = 90;
                break;
        }

        degrees = (degrees + orientation)%360;
        ByteArrayInputStream is = null;

        try {
            is = new ByteArrayInputStream(data);
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeStream(is, null, options);
            is.close();
            is = null;

            int outWidth  = options.outWidth;
            int outHeight = options.outHeight;

            int inSampleSize = 1;
            if (outWidth < outHeight) {
                while (outWidth / inSampleSize > SHORT_SIDE_LENGTH) {
                    inSampleSize++;
                }
            } else {
                while (outHeight / inSampleSize > SHORT_SIDE_LENGTH) {
                    inSampleSize++;
                }
            }
            if (inSampleSize > 1) {
                inSampleSize -= 1;
            }

            options = new BitmapFactory.Options();
            options.inJustDecodeBounds = false;
            options.inSampleSize = inSampleSize;
            options.inScaled = false;
            options.inPreferredConfig = Bitmap.Config.ARGB_8888;
            options.inDither = true;

            is = new ByteArrayInputStream(data);
            Bitmap srcBitmap = BitmapFactory.decodeStream(is, null, options);
            is.close();
            is = null;

            outWidth = srcBitmap.getWidth();
            outHeight = srcBitmap.getHeight();

            final float scale;
            final int width;
            final int height;

            if (outWidth < outHeight) {
                scale = (float)SHORT_SIDE_LENGTH / outWidth;

                if (degrees == 90 || degrees == 270) {
                    width = LONG_SIDE_LENGTH;
                    height = SHORT_SIDE_LENGTH;
                } else {
                    width = SHORT_SIDE_LENGTH;
                    height = LONG_SIDE_LENGTH;
                }
            } else {
                scale = (float)SHORT_SIDE_LENGTH / outHeight;

                if (degrees == 90 || degrees == 270) {
                    width = SHORT_SIDE_LENGTH;
                    height = LONG_SIDE_LENGTH;
                } else {
                    width = LONG_SIDE_LENGTH;
                    height = SHORT_SIDE_LENGTH;
                }
            }

            Matrix matrix = new Matrix();
            matrix.postTranslate(-.5f*outWidth, -.5f*outHeight);
            matrix.postRotate(degrees);
            matrix.postScale(flip?-scale:scale, scale);
            matrix.postTranslate(.5f*width, .5f*height);

            bitmap = Bitmap.createBitmap(width, height, Config.ARGB_8888);
            Canvas canvas = new Canvas(bitmap);
            Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG);

            canvas.drawBitmap(srcBitmap, matrix, paint);
            srcBitmap.recycle();
            srcBitmap = null;

        } catch (Exception e) {
            // Does nothing
        } finally {
            IOUtils.closeSilently(is);
        }

        return bitmap;
    }
    */
}
