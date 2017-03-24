/*
* CREATED BY: Luke Ender
* ORGANIZATION: University of Arizona, College of Optical Sciences
* PURPOSE: This program is designed send live gyroscope and accelerometer data to a simple TCP server
* REFERENCES: This program heavily relies on code published by Simen Andresen,
*             found at (http://simena86.github.io/blog/2013/04/30/logging-accelerometer-from-android-to-pc/)
 */
package com.hoopla.socket;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.Enumeration;

import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;


public class MainActivity extends Activity {

    public int PORT = 8090;
    private Button connectPhones;
    private Button streamVideo;
    private Button showinfo;
    private Button ffmpeg_control;
    private String serverIpAddress = "104.168.29.115"; //"192.168.43.67";
    private boolean connected = false;
    private boolean gyroscope = false;
    private boolean ffmpeg = false;
    TextView text_g;
    TextView text_a;
    EditText port;
    EditText ipAdr;
    EditText alpha;
    private float[] g = new float[] {0,0,0};
    private float[] a = new float[] {0,0,0};
    private SensorManager sensorManager;
    private Sensor sensor_g;
    private Sensor sensor_a;
    boolean acc_disp = false;
    PrintWriter out;
    Toast toast;
    CharSequence text;
    int duration;
    Context context;
    LowPassFilter lowPassFilter = new LowPassFilter();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        PackageManager manager = getPackageManager();
        gyroscope = manager.hasSystemFeature(PackageManager.FEATURE_SENSOR_GYROSCOPE);
        connectPhones = (Button)findViewById(R.id.send);
        streamVideo = (Button)findViewById(R.id.streamVideo);
        showinfo = (Button)findViewById(R.id.showinfo);
        ffmpeg_control = (Button)findViewById(R.id.ffmpeg);
        connectPhones.setOnClickListener(connectListener);
        streamVideo.setOnClickListener(streamListener);
        ffmpeg_control.setOnClickListener(ffmpegListener);
        showinfo.setOnClickListener(showinfoListener);
        text_g=(TextView)findViewById(R.id.textin_g);
        text_a=(TextView)findViewById(R.id.textin_a);
        port=(EditText)findViewById(R.id.port);
        ipAdr=(EditText)findViewById(R.id.ipadr);
        sensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
        if (gyroscope) {
            sensor_g = sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        }
        sensor_a = sensorManager.getDefaultSensor(Sensor.TYPE_LINEAR_ACCELERATION);
        port.setText("8090");
        ipAdr.setText(serverIpAddress);
        acc_disp =false;
    }

    private Button.OnClickListener streamListener = new Button.OnClickListener() {
        @Override
        public void onClick(View v) {

            Context context = getApplicationContext();
            CharSequence text;
            int duration;
            String streamurl = "rtmp://104.168.29.115/live/test";
            ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
            ClipData clip = ClipData.newPlainText("stream URL", streamurl);
            clipboard.setPrimaryClip(clip); // This code copies the stream address into the clipboard so all users need to do is paste and go
            if (getPackageManager().getLaunchIntentForPackage("org.videolan.vlc") != null) {
                Intent launchIntent = getPackageManager().getLaunchIntentForPackage("org.videolan.vlc");
                startActivity(launchIntent); //launch VLC
                text = "Go to streaming -> paste address from clipboard";
                duration = Toast.LENGTH_SHORT;
                toast = Toast.makeText(context, text, duration);
            }
            else {
                context = getApplicationContext();
                text = "Please install VLC for Android (available free on the Play store)";
                duration = Toast.LENGTH_LONG;
                toast = Toast.makeText(context, text, duration);
            }
            toast.show();

        }
    };

    private Button.OnClickListener ffmpegListener = new Button.OnClickListener(){
        @Override
        public void onClick(View v){ //this listener displays the application information
            context = getApplicationContext();
            duration = Toast.LENGTH_SHORT;
            if (connected){
                if (ffmpeg){
                    out.printf("stop");
                    text = "Stop signal sent";
                }
                else{
                    out.printf("start");
                    text = "Start signal sent";
                }
                out.flush();
                ffmpeg = !ffmpeg;
            }
            else{
                text = "No connection to host";
            }
            toast = Toast.makeText(context, text, duration);
            toast.show();
        }
    };

    private Button.OnClickListener showinfoListener = new Button.OnClickListener(){
        @Override
        public void onClick(View v){ //this listener displays the application information
                                                               //screen if the button for it is pressed
            Intent myIntent = new Intent(MainActivity.this, ShowInfoActivity.class);
            MainActivity.this.startActivity(myIntent);
        }
    };
    private Button.OnClickListener connectListener = new Button.OnClickListener() {
        @Override
        public void onClick(View v) { //this listener starts the thread which contains the socket
                                                              //comms code
            if (!connected) {
                if (!gyroscope){
                    context = getApplicationContext();
                    text = "No gyroscope present in phone! This app requires a gyroscope.";
                    duration = Toast.LENGTH_LONG;
                    toast = Toast.makeText(context, text, duration);
                    toast.show();
                }

               else if (!serverIpAddress.equals("")) {
                    connectPhones.setText("Stop Streaming");
                    Thread cThread = new Thread(new ClientThread());
                    cThread.start();
                }
            }
            else{
                connectPhones.setText("Start Streaming");
                connected=false;
                acc_disp=false;

            }
        }
    };
    private Button.OnClickListener showinfoListerner = new Button.OnClickListener() {
        @Override
        public void onClick(View v) {
            float inter = Float.valueOf(alpha.getText().toString());
            lowPassFilter.setFilterCoefficient(inter);
        }
    };

    public class ClientThread implements Runnable {//this thread runs separately than the UI thread and
                                                   //does all the TCP socket communications
        Socket socket;
        public void run() {
            try {
                acc_disp=true;
                PORT = Integer.parseInt(port.getText().toString());
                serverIpAddress=ipAdr.getText().toString();
                InetAddress serveraddr = InetAddress.getByName(serverIpAddress);
                serverIpAddress = serveraddr.getHostAddress(); // code up until this point reads text strings from text boxes
                socket = new Socket(serveraddr, PORT);
                connected = true;
                out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())), true);
                out.printf("phone");
                while (connected) {
                        out.printf("%10.2f,%10.2f\n", g[0], g[1]);
                        out.flush();
                        Thread.sleep(5);
                } //the above code opens a TCP socket and fires a string containing all axes of gyro and accelerometer into it
            }
            catch (IOException e) {
                e.printStackTrace();
            }
            catch (InterruptedException e) {
                e.printStackTrace();
            } finally{
                try{
                    acc_disp=false;
                    connected=false;
                    connectPhones.setText("Start Streaming");
                    socket.close();
                }catch(Exception a){
                }
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        sensorManager.registerListener(gyroscopeListener, sensor_g,
                    SensorManager.SENSOR_DELAY_FASTEST);

        sensorManager.registerListener(accelerometerListener, sensor_a,
                SensorManager.SENSOR_DELAY_FASTEST);
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    private SensorEventListener gyroscopeListener = new SensorEventListener() {

        @Override
        public void onAccuracyChanged(Sensor sensor, int acc) {
        }
        @Override
        public void onSensorChanged(SensorEvent event_g) {
            for (int i = 0; i < 3; i++) {
                g[i] = event_g.values[i];
            }
            refreshDisplay();
        }
    };

    private SensorEventListener accelerometerListener = new SensorEventListener() {

        @Override
        public void onAccuracyChanged(Sensor sensor, int acc) {
        }
        @Override
        public void onSensorChanged(SensorEvent event_a) {
            for (int i = 0; i < 3; i++) {
                a[i] = event_a.values[i];
            }
          //  a = lowPassFilter.addSamples(a); //low pass filter filters out gravity from accelermeter data
        }
    };

    private void refreshDisplay() { //This function outputs the current output string value
                                    //to the textview text_a if acc_disp if true
        if(acc_disp == true){
            String output_a = String.format("%10.2f,%10.2f,%10.2f,%10.2f,%10.2f,%10.2f\n", g[0], g[1], g[2], a[0], a[1], a[2]);
            text_a.setText(output_a);
        }
    }
}