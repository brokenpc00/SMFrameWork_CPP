package com.brokenpc.SMCocosLib;

/**
 * Created by stevemac on 2017. 10. 17..
 */

import android.app.Activity;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.util.Log;
import android.view.Surface;

import java.util.ArrayList;
import java.util.List;

@SuppressWarnings("deprecation")
public class CameraUtil {
    private static final String Tag = "CameraUtil";

    public static Size getOptimalPreviewSize(int cameraId, Parameters parameters, int displayWidth, int displayHeight) {
        final double ASPECT_TOLERANCE = 0.25;
        Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;

        List<Size> sizes = parameters.getSupportedPreviewSizes();
        int reqWidth = displayWidth;
        int reqHeight = displayHeight;

        Log.i("CAMERA UTIL", "[[[[[ get Optimal Preview Size req width : " + Integer.toString(reqWidth) + ", height : " + Integer.toString(reqHeight));

        // 1) 화면사이즈와 동일한 프리뷰 사이즈 를 찾는다
        for (Size size : sizes) {
            if(size.width == reqWidth && size.height == reqHeight) {
                optimalSize = size;
                break;
            }
        }


        // 2) height가 같은것중에서 화면 비율과 가장 가까운것을 찾는다.
        if (optimalSize == null) {
            Log.i("CAMERA UTIL", "[[[[[ get Optimal Preview Size not found 1");
            double targetRatio =  (double)reqWidth / reqHeight;
            double mindiff = Double.MAX_VALUE;
            for (Size size : sizes) {
                if (size.height == reqHeight) {
                    double ratio = (double)size.width / size.height;
                    double diff = Math.min(mindiff, targetRatio-ratio);
                    if (diff < mindiff) {
                        optimalSize = size;
                        minDiff = diff;
                    }
                }
            }
        } else {
            Log.i("CAMERA UTIL", "[[[[[ get Optimal Preview Size find support width : " + Integer.toString(optimalSize.width) + ", height : " + Integer.toString(optimalSize.height));
        }

        // 3) 화면 비율과 같은 프리뷰 사이즈를 찾는다
        if (optimalSize == null) {
            Log.i("CAMERA UTIL", "[[[[[ get Optimal Preview Size not found 2");
            double targetRatio =  (double)reqWidth / reqHeight;
            for (Size size : sizes) {
                double ratio = (double)size.width / size.height;
                if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) continue;
                if (Math.abs(size.width - reqWidth) < minDiff) {
                    optimalSize = size;
                    minDiff = Math.abs(size.width - reqWidth);
                }
            }
        } else {
            Log.i("CAMERA UTIL", "[[[[[ get Optimal Preview Size find ratio 1 width : " + Integer.toString(optimalSize.width) + ", height : " + Integer.toString(optimalSize.height));
        }

        // 4) 없으면.. 비슷한 사이즈로 찾는다.
        if (optimalSize == null) {
            //Log.v(TAG, "No preview size match the aspect ratio");
            Log.i("CAMERA UTIL", "[[[[[ get Optimal Preview Size not found 3");
            minDiff = Double.MAX_VALUE;
            for (Size size : sizes) {
                if (Math.abs(size.width - reqWidth) < minDiff) {
                    optimalSize = size;
                    minDiff = Math.abs(size.height - reqWidth);
                }
            }
        } else {
            Log.i("CAMERA UTIL", "[[[[[ get Optimal Preview Size find ratio 2 width : " + Integer.toString(optimalSize.width) + ", height : " + Integer.toString(optimalSize.height));
        }

        if (optimalSize!=null) {
            Log.i("CAMERA UTIL", "[[[[[ get Optimal Preview Size find ratio 3 width : " + Integer.toString(optimalSize.width) + ", height : " + Integer.toString(optimalSize.height));
        } else {
            Log.i("CAMERA UTIL", "[[[[[ get Optimal Preview Size not found 4");
        }

        // 5) 폭/높이 모두 1000px가 넘으면 같은 비율중 1000 이하를 찾는다.
        if (optimalSize.width > 1000 && optimalSize.height > 1000) {
            double targetRatio =  (double)optimalSize.width / optimalSize.height;
            Size targetSize = null;
            for (Size size : sizes) {
                if (size.width < 1000 || size.height < 1000) {
                    double ratio = (double)size.width/size.height;
                    if (Math.abs(ratio - targetRatio) < 0.01) {
                        if (targetSize == null || size.width > targetSize.width) {
                            targetSize = size;
                        }
                    }
                }
            }
            if (targetSize != null) {
                optimalSize = targetSize;
            }
            Log.i("CAMERA UTIL", "[[[[[ get Optimal Preview Size find under 1000 px width : " + Integer.toString(optimalSize.width) + ", height : " + Integer.toString(optimalSize.height));
        }

        return optimalSize;
    }

    public static Size getOptimalPictureSize(Parameters parameters, Size previewSize) {
        final double ASPECT_TOLERANCE = 0.25;
        Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;

        List<Size> sizes = parameters.getSupportedPictureSizes();
        checkSupportedPictureSizeAtPreviewSize(parameters, sizes);

        int previewWidth  = previewSize.width;
        int previewHeight = previewSize.height;

        ArrayList<Size> candidateSizes = new ArrayList<>();
        Size adaptiveSize = null;

        // 1) 프리뷰 비율과 동일한 Picture 사이즈를 찾는다.
        double previewRatio =  (double)previewWidth / previewHeight;
        for (Size size : sizes) {
            double ratio = (double)size.width / size.height;
            if (Math.abs(ratio - previewRatio) > ASPECT_TOLERANCE) continue;
            candidateSizes.add(size);
        }

        // 2) 프리뷰 보다 크고 1024가 넘는 Picture 사이즈중 가장 작은것을 찾는다.
        Size equalSize = null;
        minDiff = Double.MAX_VALUE;
        for (Size size : candidateSizes) {
            if(size.width < previewWidth || size.width < 1024)
                continue;
            else if(size.width == previewWidth) {
                if (size.height == previewHeight) {
                    equalSize = size;
                } else if (equalSize == null) {
                    equalSize = size;
                }
            }
            else if(Math.abs(size.width - previewWidth) < minDiff) {
                minDiff = Math.abs(previewWidth - size.width);
                optimalSize = size;
            }
        }


        if (equalSize != null && equalSize.width > 1024) {
            optimalSize = equalSize;
        }

        // 3) 프리뷰 보다 큰 Picture 사이즈중 가장 작은것을 찾는다.
        if (optimalSize == null) {
            minDiff = Double.MAX_VALUE;
            for (Size size : candidateSizes) {
                if(size.width < previewWidth)
                    continue;
                else if(size.width == previewWidth) {
                    equalSize = size;
                }
                else if(Math.abs(size.width - previewWidth) < minDiff) {
                    minDiff = Math.abs(previewWidth - size.width);
                    optimalSize = size;
                }
            }
        }

        int picWidth  = 1024;
        int picHeight = 768;

        // 4) optimal이 있더라도 1024x768 보다 큰사이즈가 있으면 찾는다.
        if (optimalSize != null &&
                (optimalSize.width < picWidth || optimalSize.height < picHeight)) {
            Size optimal2 = null;
            minDiff = Double.MAX_VALUE;
            for (Size size : candidateSizes) {
                if(size.width < picWidth || size.height < picHeight)
                    continue;

                else if(size.width == previewWidth) {
                    equalSize = size;
                }
                else if(Math.abs(size.width - previewWidth) < minDiff) {
                    minDiff = Math.abs(previewWidth - size.width);
                    optimal2 = size;
                }
            }

            if (optimal2 != null) {
                optimalSize = optimal2;
            }
        }

        // 5) optimalSize 찾지 못했으면 4:3 사이즈에서 1024x1024 넘는 것중 가장 작은것을 찾는다.
        if (optimalSize == null) {
            for (Size size : sizes) {
                double ratio = (double)size.width / size.height;
                if (Math.abs(ratio - 4/3f) > ASPECT_TOLERANCE) continue;
                if (size.width > 1024 && size.height > 1024) {
                    if (adaptiveSize != null) {
                        if (size.width < adaptiveSize.width) {
                            adaptiveSize = size;
                        }
                    } else {
                        adaptiveSize = size;
                    }
                }
            }
        }


        // 6) 큰것이 없으면.. 동일한 사이즈라도 찾는다.
        if(optimalSize == null) {
            if(equalSize != null) {
                if (adaptiveSize != null && equalSize.width < 1024) {
                    optimalSize = adaptiveSize;
                } else {
                    optimalSize = equalSize;
                }
            } else {
                optimalSize = adaptiveSize;
            }
        }


        // 7) 그래도 없으면..
        if(optimalSize == null) {
            optimalSize = sizes.get(0);
        }

        return optimalSize;
    }

    private static void checkSupportedPictureSizeAtPreviewSize(Parameters parameters, List<Size> pictureSizes) {
        List<Size> previewSizes = parameters.getSupportedPreviewSizes();
        Camera.Size pictureSize;
        Camera.Size previewSize;
        double  pictureRatio = 0;
        double  previewRatio = 0;
        final double aspectTolerance = 0.05;
        boolean isUsablePicture = false;

        for (int indexOfPicture = pictureSizes.size() - 1; indexOfPicture >= 0; --indexOfPicture) {
            pictureSize = pictureSizes.get(indexOfPicture);
            pictureRatio = (double) pictureSize.width / (double) pictureSize.height;
            isUsablePicture = false;

            for (int indexOfPreview = previewSizes.size() - 1; indexOfPreview >= 0; --indexOfPreview) {
                previewSize = previewSizes.get(indexOfPreview);

                previewRatio = (double) previewSize.width / (double) previewSize.height;

                if (Math.abs(pictureRatio - previewRatio) < aspectTolerance) {
                    isUsablePicture = true;
                    break;
                }
            }

            if (isUsablePicture == false) {
                pictureSizes.remove(indexOfPicture);
            }
        }
    }

    private static int getDisplayRotation(Activity activity) {
        int rotation = activity.getWindowManager().getDefaultDisplay()
                .getRotation();
        switch (rotation) {
            case Surface.ROTATION_0: return 0;
            case Surface.ROTATION_90: return 90;
            case Surface.ROTATION_180: return 180;
            case Surface.ROTATION_270: return 270;
        }
        return 0;
    }
}

