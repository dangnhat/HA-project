package anh.trinh.ble_demo;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

public class ScenesFragment extends Fragment {
 @Override
public View onCreateView(LayoutInflater inflater, ViewGroup container,
		Bundle savedInstanceState) {
	// TODO Auto-generated method stub
	 View rootView = inflater.inflate(R.layout.fragment_scenes, container, false);
	return super.onCreateView(inflater, container, savedInstanceState);
}
}
