package com.brokenpc.SMCocosLib;

import android.app.Activity;
import android.content.Context;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;


public class GPSManager {
    private final String TAG = getClass().getSimpleName();
    private Activity m_activity = null;

    private LocationManager m_locManager = null;

    private CallbackLocation m_callbackEvent;

    public GPSManager(Activity activity, CallbackLocation event) {
        m_activity = activity;
        m_callbackEvent = event;
        m_locManager = (LocationManager) m_activity.getSystemService(Context.LOCATION_SERVICE);
    }

    public void startGPS() {

        boolean isGPSEnable = m_locManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
        boolean isNetEnable = m_locManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);

        if (!isGPSEnable && !isNetEnable) {
            return;
        }

        Criteria reqment = new Criteria();
        reqment.setAccuracy(Criteria.ACCURACY_FINE);
        reqment.setAltitudeRequired(false);
        reqment.setBearingRequired(false);
        reqment.setCostAllowed(true);
        reqment.setPowerRequirement(Criteria.POWER_LOW);

        String pvder = m_locManager.getBestProvider(reqment, true);
        Location loc = m_locManager.getLastKnownLocation(pvder);
        m_callbackEvent.callbackMethod(loc);
        m_locManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 10000, 5, locListener);
        m_locManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, 10000, 5, locListener);
    }

    public void stopGPS() {
        m_locManager.removeUpdates(locListener);
    }

    private final LocationListener locListener = new LocationListener() {
        @Override
        public void onLocationChanged(Location location) {
            // TODO Auto-generated method stub
            m_callbackEvent.callbackMethod(location);
        }

        public void onProviderDisabled(String provider) {
            m_callbackEvent.callbackMethod(null);
        }

        public void onProviderEnabled(String provider) {
        }

        public void onStatusChanged(String provider, int status, Bundle extra) {
        }
    };
}
