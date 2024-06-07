package abc.cookie.plugin;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import abc.cookie.plugin.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'plugin' library on application startup.
    static {
        System.loadLibrary("plugin");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
    }
}