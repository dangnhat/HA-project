package anh.trinh.ble_demo;

import android.media.Image;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageButton;

public class ScenesFragment extends Fragment {
		
	private Button btnAddScene;
	 @Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		 View rootView = inflater.inflate(R.layout.fragment_scenes, container, false);
		
		btnAddScene = (Button) rootView.findViewById(R.id.btnAddScene);
		
		btnAddScene.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Log.i("ButtonAdd", "Add");
			}
		});
		
		return rootView;
	}
}
