package anh.trinh.ble_demo;

import java.nio.ByteBuffer;
import java.sql.Array;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

import org.w3c.dom.ls.LSInput;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.ListFragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.ExpandableListView;
import android.widget.ExpandableListView.OnChildClickListener;
import android.widget.ExpandableListView.OnGroupClickListener;
import android.widget.ExpandableListView.OnGroupCollapseListener;
import android.widget.ExpandableListView.OnGroupExpandListener;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;
import anh.trinh.ble_demo.data.BluetoothMessage;
import anh.trinh.ble_demo.data.CommandID;
import anh.trinh.ble_demo.data.DataConversion;
import anh.trinh.ble_demo.data.DeviceInfo;
import anh.trinh.ble_demo.data.DeviceTypeDef;
import anh.trinh.ble_demo.data.ProcessBTMsg;
import anh.trinh.ble_demo.list_view.Device_c;
import anh.trinh.ble_demo.list_view.ExpandableListViewAdapter;
import anh.trinh.ble_demo.list_view.Zone_c;

public class DeviceControlFragment extends Fragment{
	private ExpandableListView 	lvDevControl;
	private ExpandableListViewAdapter mAdapter;
	private ArrayList<Zone_c> 	listParent 		= new ArrayList<Zone_c>();
	private final String 		TAG				= "DeviceControlFragment";
	private HomeActivity        mHomeActivity   = (HomeActivity) getActivity();

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		View rootView = inflater.inflate(R.layout.fragment_device_control, container, false);
		
		lvDevControl	= (ExpandableListView) rootView
										.findViewById(R.id.elvDeviceControl);
		
		Log.i(TAG, "enter process");
		
		mAdapter = new ExpandableListViewAdapter((HomeActivity) getActivity(), listParent);
		lvDevControl.setGroupIndicator(null);
		lvDevControl.setAdapter(mAdapter);
		
		return rootView;
	}
	
	
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		Log.i(TAG, "onActivity create");
	}
	
	@Override
	public void onDestroyView() {
		// TODO Auto-generated method stub
		super.onDestroyView();
		Log.i(TAG, "onDestroyView");
	}
	
	
	
	public void updateUI(ArrayList<DeviceInfo> deviceList){
//		Log.i(TAG, Integer.toString(deviceList.size()) );
		prepareDataForDisp(deviceList);
		if(!listParent.isEmpty()){
			Log.i(TAG, "data available");
//			lvDevControl.invalidateViews();
			mAdapter.notifyDataSetChanged();
		}
		
		for(int i = 0; i < mAdapter.getGroupCount(); i++){
			if(!lvDevControl.isGroupExpanded(i)){
				lvDevControl.expandGroup(i);
			}
		}
	}
	
	/**
	 * ininial virtual data 
	 */
	private void initialStaticData(){
		ArrayList<DeviceInfo> mDevList = new ArrayList<DeviceInfo>();

        // initial static device list for expandable listview
      for(int i = 0; i < 8; i++){
    	DeviceInfo mDevInfo = new DeviceInfo();
      	switch (i) {
			case 0:
				mDevInfo.setDevID(0x00000003);
				mDevInfo.setDevVal((short) 0);
				break;
			case 1:
				mDevInfo.setDevID(0x00000144);
				mDevInfo.setDevVal((short) 0);
				break;
			case 2:
				mDevInfo.setDevID(0x000000041);
				mDevInfo.setDevVal((short) 0);
				break;
			case 3:
				mDevInfo.setDevID(0x00000102);
				mDevInfo.setDevVal((short) 0);
				break;
			case 4:
				mDevInfo.setDevID(0x00000205);
				mDevInfo.setDevVal((short) 0);
				break;
			case 5:
				mDevInfo.setDevID(0x00000107);
				mDevInfo.setDevVal((short) 0);
				break;
			case 6:
				mDevInfo.setDevID(0x00000042);
				mDevInfo.setDevVal((short) 0);
				break;
			case 7:
				mDevInfo.setDevID(0x00000207);
				mDevInfo.setDevVal((short) 0);
				break;
			default:
				mDevInfo.setDevID(0x00000107);
				mDevInfo.setDevVal((short) 0);
				break;
			}
      	mDevList.add(mDevInfo);
      }
		
		ByteBuffer devID = ByteBuffer.allocate(4);
		byte mZoneId;
		byte mDevId;
		Zone_c zone = null;
		
		for (int i = 0; i < mDevList.size(); i++){
			Log.i("prepareData", "hello" );
//			Log.i("prepare_data", Byte.toString(devID.putInt(mDevList.get(i).getDevID()).get(2)) );
//			Log.i("prepare_data", Integer.toString(devID.putInt(mDevList.get(i).getDevID()).get(3)) );
//			if(false == searchZone(listParent, zone)){
//				listParent.add(zone);
//				Log.i("prepare_data","hello");
//			}
			
			mZoneId = devID.putInt(mDevList.get(i).getDevID()).get(2);
			devID.clear();
			mDevId = devID.putInt(mDevList.get(i).getDevID()).get(3);
			devID.clear();
			
			if(!searchZone(listParent, mZoneId)){
				zone = new Zone_c();
				zone.setName(mZoneId);
				listParent.add(zone);
			}
			
			Device_c device = new Device_c();
			device.setName(mDevId);
			device.setVal(mDevList.get(i).getDevVal());
			listParent.get(getZoneIndex(listParent, mZoneId)).addChildListItem(device);
		}
	}
	
	
	/**
	 * Initial data for Expandable ListView
	 */
	private void prepareDataForDisp(ArrayList<DeviceInfo> deviceList){
		ArrayList<DeviceInfo> mDevList =  new ArrayList<DeviceInfo>();
		mDevList = 	deviceList;
		Log.i("DeviceList size", Integer.toString(mDevList.size()));
		
		ByteBuffer devID = ByteBuffer.allocate(4);
		int mZoneId;
		int mDevId;
		Zone_c zone = null;
		listParent.clear();
		for (int i = 0; i < mDevList.size(); i++){
//			mZoneId = devID.putInt(mDevList.get(i).getDevID()).get(0);
			mZoneId = DataConversion.byteType( devID.putInt(mDevList.get(i).getDevID()).get(0) );
			Log.i(TAG, Integer.toString(mZoneId));
			devID.clear();
			mDevId = devID.putInt(mDevList.get(i).getDevID()).get(3);
			devID.clear();
			
			if(!searchZone(listParent, mZoneId)){
				zone = new Zone_c();
				zone.setName(mZoneId);
				listParent.add(zone);
			}
			
			Device_c device = new Device_c();
			device.setName((byte) mDevId);
			device.setVal(mDevList.get(i).getDevVal());
			listParent.get(getZoneIndex(listParent, mZoneId)).addChildListItem(device);
		}
		
	}
	
	/**
	 * Search zone by ID
	 * 
	 * @param parentList
	 * @param item
	 * @return
	 */
	private boolean searchZone(ArrayList<Zone_c> parentList, int mZoneId){
		if(parentList.isEmpty() ){
			return false;
		}
		for(Zone_c parent : parentList){
			if(parent.getID() == mZoneId){
				return true;
			}
		}
		return false;
	}
	
	/**
	 * Get zone index in List
	 * 
	 * @param zoneList
	 * @param mZoneId
	 * @return
	 */
	private int getZoneIndex(ArrayList<Zone_c> zoneList, int mZoneId){
		
		for(int i = 0; i< zoneList.size(); i++){
			if(zoneList.get(i).getID() == mZoneId){
				return i;
			}
		}
		return -1;
	}
	
}
