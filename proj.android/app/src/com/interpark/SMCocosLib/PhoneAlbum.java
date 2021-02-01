package com.brokenpc.SMCocosLib;

/**
 * Created by stevemac on 2017. 10. 17..
 */

import java.util.Vector;

public class PhoneAlbum {
    private int id;
    private long bucketId;
    private int albumIndex;
    private String name;
    private String coverUri;    // 대표 이미지 file path
    private int photoCount;
    private Vector<PhonePhoto> albumPhotos;

    public int getId() {
        return id;
    }

    public int getAlbumIndex() {return  albumIndex;}

    public void setId(int id) {
        this.id = id;
    }

    public long getBucketId() {return bucketId;}

    public void setBucketId(long bucketId) {this.bucketId = bucketId;}

    public void setAlbumIndex(int albumIndex) {this.albumIndex=albumIndex;}

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getCoverUri() {
        return coverUri;
    }

    public void setCoverUri(String coverUri) {
        this.coverUri = coverUri;
    }

    public int getPhotoCount() {return photoCount;}

    public void setPhotoCount(int photoCount) {this.photoCount = photoCount;}

    public Vector<PhonePhoto> getAlbumPhotos() {
        if (albumPhotos==null) {
            albumPhotos = new Vector<>();
        }
        return albumPhotos;
    }

    public void setAlbumPhotos(Vector<PhonePhoto> albumPhotos) {
        this.albumPhotos = albumPhotos;
    }

}

