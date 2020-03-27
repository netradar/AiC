package com.vanxum.Aic;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;


import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import com.vanxum.Aic.R;

public class Ipconnect extends Activity {

    EditText ipaddr,port;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ipconnect);

        ipaddr = (EditText)findViewById(R.id.peer);
        port = (EditText)findViewById(R.id.peerIdentify);

        final SharedPreferences aic = getApplicationContext().getSharedPreferences("aic", MODE_PRIVATE);
        String i = aic.getString("ipaddr","");
        String p = aic.getString("port","");

        ipaddr.setText(i);
        port.setText(p);

        /*String contacts = settings.getString("contacts","[]");
        try {
            JSONArray array = new JSONArray(contacts);
            JSONObject object;
            for (int i = 0; i < array.length(); i++) {
                object = array.getJSONObject(i);
                Log.e(TAG,"contacts address:" + object.getString("address") + " note:" + object.getString("note"));
            }

        } catch (JSONException e) {
            e.printStackTrace();
        }*/




    }


    public void onBtnClick(View v)
    {


        final String sIp = ipaddr.getText().toString();
        final String sPort = port.getText().toString();

        if(sIp.isEmpty()||sPort.isEmpty())
        {
            final Toast toast = Toast.makeText(Ipconnect.this, "IP地址和端口号不能为空", Toast.LENGTH_SHORT);
            toast.show();
            return;
        }

        final Toast toast = Toast.makeText(Ipconnect.this, "连接中", Toast.LENGTH_SHORT);
        toast.show();

        final SharedPreferences aic = getApplicationContext().getSharedPreferences("aic", MODE_PRIVATE);
        aic.edit().putString("ipaddr",sIp)
                .putString("port",sPort).commit();

        Intent intent = new Intent();
        intent.putExtra("ipaddr",sIp);
        intent.putExtra("port",Integer.valueOf(sPort));
        intent.setClass(Ipconnect.this, RenderBoard.class);
        startActivity(intent);
    }


}