package com.brokenpc.SMCocosLib;

/**
 * Created by stevemac on 2017. 10. 17..
 */

import java.util.Vector;

public interface OnPhoneImagesObtained {
    void onLoadAlbumComplete(Vector<PhoneAlbum> albums);
    void onError();
}
