package anh.trinh.ble_demo;

import java.util.ArrayList;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.ExpandableListView;
import anh.trinh.ble_demo.list_view.Child_c;
import anh.trinh.ble_demo.list_view.ExpandableListViewAdapter;
import anh.trinh.ble_demo.list_view.Parent_c;

public class DeviceControlFragment extends Fragment{
	private ExpandableListView 		lvDevControl;
	private ArrayList<Parent_c> 	listParent 	= new ArrayList<Parent_c>();
	private ArrayList<Child_c> 		listChild 	= new ArrayList<Child_c>();
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		View rootView = inflater.inflate(R.layout.fragment_device_control, container, false);
		
		lvDevControl	= (ExpandableListView) rootView
										.findViewById(R.id.elvDeviceControl);
		create_dummy_data();
		ExpandableListViewAdapter mAdapter = new ExpandableListViewAdapter(	getActivity(), 
																			listParent, 
																			listChild	);
		lvDevControl.setGroupIndicator(null);
		lvDevControl.setAdapter(mAdapter);
		return rootView;
	}
	
	private void create_dummy_data(){
		String roomName[] 	= {"Kitchen", "Bathroom", "Living room", "Bedroom"};
		String devName[]	= {"Bulb", "Dimmer", "Motor", "Sensor"};
		
		for(int i = 0 ; i < devName.length; i++){
			Child_c child 	= new Child_c(devName[i], 0);
			listChild.add(child); 
		}
		
		for(int i = 0 ; i < roomName.length; i++){
			Parent_c parent = new Parent_c(roomName[i], listChild);
			listParent.add(parent);
		}
		
	}
}
