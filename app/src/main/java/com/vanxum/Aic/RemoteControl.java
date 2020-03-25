package com.vanxum.Aic;

import androidx.appcompat.app.AppCompatActivity;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.Window;

public class RemoteControl extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_remote_control);

        Window window = getWindow();
        window.setBackgroundDrawableResource(android.R.color.transparent);
    }



    public void onMenuClick(View v)
    {
        if(v==findViewById(R.id.sendBack))
        {
            setResult(0);
            finish();
        }
        if(v==findViewById(R.id.sendHome))
        {
            setResult(1);
            finish();
        }
        if(v==findViewById(R.id.sendMenu))
        {
            setResult(2);
            finish();
        }
        if(v==findViewById(R.id.sendDisconnect))
        {
            setResult(3);
            finish();

        }

    }
}
