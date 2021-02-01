package com.brokenpc.SMCocosLib;

/**
 * Created by stevemac on 2017. 10. 17..
 */

public class PhonePhoto {
    private int id; // id
    private int photoIndex;
    private String albumName;   // 소속된 앨범명 ios와 다르게 앨범이 directory명이므로 하나의 앨범만 가지고 있다.
    private String photoUri;    // local file path
    private int orientation;

    public int getId() {
        return id;
    }

    public int getPhotoIndex() {return photoIndex;}

    public void setId(int id) {
        this.id = id;
    }

    public void setPhotoIndex(int photoIndex) {this.photoIndex=photoIndex;}

    public String getAlbumName() {
        return albumName;
    }

    public void setAlbumName(String name) {
        this.albumName = name;
    }

    public String getPhotoUri() {
        return photoUri;
    }

    public void setPhotoUri(String photoUri) {
        this.photoUri = photoUri;
    }

    public int getOrientation() {return orientation;}

    public void setOrientation(int orientation) {this.orientation = orientation;}
}
