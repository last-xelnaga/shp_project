package com.xelnaga.shp.client;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

import com.google.firebase.iid.FirebaseInstanceId;
import com.google.firebase.iid.FirebaseInstanceIdService;


public class ServiceFirebaseInstanceId extends FirebaseInstanceIdService {

    private static final String TAG = "ServiceFirebaseInstanceId";

    /**
     * Called if InstanceID token is updated. This may occur if the security of
     * the previous token had been compromised. Note that this is called when the InstanceID token
     * is initially generated so this is where you would retrieve the token.
     */
    @Override
    public void onTokenRefresh() {
        Log.d(TAG, "onTokenRefresh activated");

        // If you want to send messages to this application instance or
        // manage this apps subscriptions on the server side, send the
        // Instance ID token to your app server.
        sendRegistrationToServer(this);
    }

    /**
     * Persist token to third-party servers.
     *
     * Modify this method to associate the user's FCM InstanceID token with any server-side account
     * maintained by your application.
     *
     * @param context Context.
     */
    public static void sendRegistrationToServer(Context context) {

        // Get updated InstanceID token.
        String refreshedToken = FirebaseInstanceId.getInstance().getToken();

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        Boolean switchPref = preferences.getBoolean ("check_box_preference_1", false);

        //Toast.makeText(this, switchPref.toString(), Toast.LENGTH_SHORT).show();
        if (switchPref) {

            String message = "{ \n";
            message += "  \"client\" : \"android\",\n";
            message += "  \"evt_time\" : 123,\n";
            message += "  \"evt_time_unix\" : 123,\n";
            message += "  \"type\" : \"token_id\",\n";
            message += "  \"data\" : {\n";
            message += "    \"token\" : \"" + refreshedToken + "\"\n";
            message += "  }\n";
            message += "}";

            String address = preferences.getString("edit_text_preference_1", "");
            ClientSocket myClient = new ClientSocket(address, 5000, message);
            myClient.execute();
        }
    }
}
