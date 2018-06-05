package com.xelnaga.shp.client;

import android.os.AsyncTask;
import android.util.Log;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;

public class ClientSocket extends AsyncTask<Void, Void, Void> {

    private static final String TAG = "ClientSocket";
    private static final int SLEEP_TIME = 50;

    String addr;
    int port;
    String message = "";
    String response = "";

    private OutputStream mOutStream = null;
    private InputStream mInStream = null;

    public static int convert(byte[] data) {
        int result = 0;

        result += data[3];
        result <<= 8;
        result += data[2];
        result <<= 8;
        result += data[1];
        result <<= 8;
        result += data[0];

        return result;
    }

    private static byte[] convert (int value) {
        byte[] data = new byte[4];

        data [0] = (byte)((value >>  0) & 0xff);
        data [1] = (byte)((value >>  8) & 0xff);
        data [2] = (byte)((value >> 16) & 0xff);
        data [3] = (byte)((value >> 24) & 0xff);

        return data;
    }

    ClientSocket(String addr, int port, String message) {

        this.addr = addr;
        this.port = port;
        this.message = message;

        Log.d(TAG, "connect to server: " + addr + ", port " + port);
        Log.d(TAG, "message:\n" + message);

    }

    @Override
    protected Void doInBackground(Void... arg0) {

        Socket socket = null;

        try {
            socket = new Socket(addr, port);

            // send message to server
            mOutStream = socket.getOutputStream();
            mOutStream.write(convert(message.length()));
            mOutStream.write(message.getBytes("UTF-8"));

            // and get the response back
            mInStream = socket.getInputStream();
            int times = 10 * 1000 / SLEEP_TIME;
            try {
                while (mInStream.available() < 3) {
                    Thread.sleep(SLEEP_TIME);
                    times --;

                    if (times == 0)
                        throw new IOException("timeout");
                }
            } catch (InterruptedException ex) {
                Log.e(TAG,"Unexpectedly got an InterruptedException: " + ex);
            }

            byte[] header = new byte[4];
            mInStream.read(header, 0, 4);
            int message_len = convert (header);
            header = null;

            Log.d(TAG, "message_len: " + message_len);

            byte[] data = new byte[message_len];
            int bytesRead = 0;
            int bytesRet = 0;
            while (bytesRead < message_len) {
                if ((bytesRet = mInStream.read(data, bytesRead, message_len - bytesRead)) <= 0) {
                    break;
                }
                bytesRead += bytesRet;
            }

            if (bytesRead != message_len) {
                data = null;
                throw new IOException("Invalid data received from daemon");
            }

            response = new String(data);
        } catch (UnknownHostException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // releases system resources associated with this stream
            if(mOutStream != null)
                try {
                    mOutStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }

            if(mInStream != null)
                try {
                    mInStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }

            if (socket != null)
                try {
                    socket.close();
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
        }

        return null;
    }

    @Override
    protected void onPostExecute(Void result) {
        Log.d(TAG, response);
        super.onPostExecute(result);
    }
}
