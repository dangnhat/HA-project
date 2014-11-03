package anh.trinh.ble_demo.list_view;

import java.util.ArrayList;
import java.util.Random;

import android.app.Activity;
import android.content.Context;
import android.graphics.drawable.ColorDrawable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView.FindListener;
import android.widget.BaseExpandableListAdapter;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.ToggleButton;
import anh.trinh.ble_demo.R;
import anh.trinh.ble_demo.R.color;
import anh.trinh.ble_demo.data.DeviceTypeDef;


public class ExpandableListViewAdapter extends BaseExpandableListAdapter {
	
	
	private Activity 				mContext;
	private ArrayList<Zone_c>		listParent;
	private class DeviceHolder{
		public int 			mHolderType;
		public TextView 	mDevName;
		public ImageView 	mDevImg;
		public TextView		mDevVal;
		public SeekBar		mSeekBar;
		public ToggleButton mBtnOnOff;
		public ImageButton	mBtnDec;
		public ImageButton	mBtnInc;
	}
	
	private class ZoneViewHolder{
		public int 		mZoneId;
		public TextView mZoneName;
		public int 		mColor;
	}
	/**
	 * Constructor 
	 */
	public ExpandableListViewAdapter(	Activity mContext, 
										ArrayList<Zone_c> listParent)
	{
		this.mContext	= mContext;
		this.listParent	= listParent;
	}

	@Override
	public Object getChild(int groupPos, int childPos) {
		// TODO Auto-generated method stub
		return listParent.get(groupPos).getChildList().get(childPos);
	}

	@Override
	public long getChildId(int groupPos, int childPos) {
		// TODO Auto-generated method stub
		return listParent.get(groupPos).getChildIndex(childPos).getID();
	}

	@Override
	public View getChildView(int groupPos, int childPos, boolean isLastChild, View convertView,
			ViewGroup parent) {
		// TODO Auto-generated method stub
		
		final Device_c childObj	= (Device_c) getChild(groupPos, childPos);
		int devType = childObj.getID();
		DeviceHolder mDevViewHolder;
		
		if(convertView == null){
			mDevViewHolder = new DeviceHolder();
			mDevViewHolder.mHolderType = devType;
			
			LayoutInflater inf 	= 	(LayoutInflater) mContext
					.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			// assign layout match with type of device
			switch (devType) {
			
			case DeviceTypeDef.BUTTON:
				convertView = inf.inflate(R.layout.switch_item, null);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mBtnOnOff = (ToggleButton)convertView.findViewById(R.id.toggleVal);
				break;
			case DeviceTypeDef.DIMMER:
				convertView = inf.inflate(R.layout.bulb, null);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mSeekBar = (SeekBar)convertView.findViewById(R.id.dimBar);
				mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
				break;
			case DeviceTypeDef.GAS_SENSOR:
				convertView = inf.inflate(R.layout.sensor_dev, null);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
				break;
			case DeviceTypeDef.LEVEL_BULB:
				convertView = inf.inflate(R.layout.bulb, null);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mSeekBar = (SeekBar)convertView.findViewById(R.id.dimBar);
				mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
				break;
			case DeviceTypeDef.LIGHT_SENSOR:
				convertView = inf.inflate(R.layout.sensor_dev, null);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
				break;
			case DeviceTypeDef.MOTOR:
				convertView = inf.inflate(R.layout.bulb, null);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mSeekBar = (SeekBar)convertView.findViewById(R.id.dimBar);
				mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
				break;
			case DeviceTypeDef.ON_OFF_BULB:
				convertView = inf.inflate(R.layout.switch_item, null);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mBtnOnOff = (ToggleButton)convertView.findViewById(R.id.toggleVal);
				
				break;
			case DeviceTypeDef.PIR_SENSOR:
				convertView = inf.inflate(R.layout.sensor_dev, null);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
				break;
			case DeviceTypeDef.RGB_LED:
				convertView = inf.inflate(R.layout.sensor_dev, null);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
				break;
			case DeviceTypeDef.SWITCH:
				convertView = inf.inflate(R.layout.switch_item, null);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mBtnOnOff = (ToggleButton)convertView.findViewById(R.id.toggleVal);
				break;
			case DeviceTypeDef.TEMP_SENSOR:
				convertView = inf.inflate(R.layout.sensor_dev, null);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
				break;
			default:
				convertView = inf.inflate(R.layout.device_list, null);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
				break;
			}
			convertView.setTag(mDevViewHolder);
		} 
		else{
			mDevViewHolder  = (DeviceHolder) convertView.getTag();
			LayoutInflater inf 	= 	(LayoutInflater) mContext
					.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			if(mDevViewHolder.mHolderType != devType){
				mDevViewHolder = new DeviceHolder();
				mDevViewHolder.mHolderType  = devType;
				
				// assign layout match with type of device
				switch (devType) {
				
				case DeviceTypeDef.BUTTON:
					convertView = inf.inflate(R.layout.switch_item, null);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mBtnOnOff = (ToggleButton)convertView.findViewById(R.id.toggleVal);
					break;
				case DeviceTypeDef.DIMMER:
					convertView = inf.inflate(R.layout.bulb, null);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mSeekBar = (SeekBar)convertView.findViewById(R.id.dimBar);
					mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.GAS_SENSOR:
					convertView = inf.inflate(R.layout.sensor_dev, null);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.LEVEL_BULB:
					convertView = inf.inflate(R.layout.bulb, null);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mSeekBar = (SeekBar)convertView.findViewById(R.id.dimBar);
					mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.LIGHT_SENSOR:
					convertView = inf.inflate(R.layout.sensor_dev, null);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.MOTOR:
					convertView = inf.inflate(R.layout.bulb, null);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mSeekBar = (SeekBar)convertView.findViewById(R.id.dimBar);
					mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.ON_OFF_BULB:
					convertView = inf.inflate(R.layout.switch_item, null);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mBtnOnOff = (ToggleButton)convertView.findViewById(R.id.toggleVal);
					
					break;
				case DeviceTypeDef.PIR_SENSOR:
					convertView = inf.inflate(R.layout.sensor_dev, null);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.RGB_LED:
					convertView = inf.inflate(R.layout.sensor_dev, null);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.SWITCH:
					convertView = inf.inflate(R.layout.switch_item, null);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mBtnOnOff = (ToggleButton)convertView.findViewById(R.id.toggleVal);
					break;
				case DeviceTypeDef.TEMP_SENSOR:
					convertView = inf.inflate(R.layout.sensor_dev, null);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				default:
					convertView = inf.inflate(R.layout.device_list, null);
					mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				};
				
				convertView.setTag(mDevViewHolder);
			}
		}
		
		switch (mDevViewHolder.mHolderType) {
		case DeviceTypeDef.BUTTON:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.button_push);
			break;
		case DeviceTypeDef.DIMMER:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.dimmer);
			break;	
		case DeviceTypeDef.GAS_SENSOR:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.gas_sensor);
			break;
		case DeviceTypeDef.LEVEL_BULB:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.bulb);
			break;
		case DeviceTypeDef.LIGHT_SENSOR:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.light_sensor);
			break;
		case DeviceTypeDef.MOTOR:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.motor);
			break;
		case DeviceTypeDef.ON_OFF_BULB:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.bulb);
			break;
		case DeviceTypeDef.PIR_SENSOR:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.pir_sensor);
			break;
		case DeviceTypeDef.RGB_LED:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.rgb_led);
			break;
		case DeviceTypeDef.SWITCH:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.switch_btn);
			break;
		case DeviceTypeDef.TEMP_SENSOR:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.temp_sensor);
			break;
			
		default:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.inknon);
			break;
		}
		
		mDevViewHolder.mDevName.setText(childObj.getName());
//		mDevViewHolder.mDevImg.setImageResource(R.drawable.inknon);
		return convertView;
	}

	@Override
	public int getChildrenCount(int groupPos) {
		// TODO Auto-generated method stub
		return listParent.get(groupPos).getChildCount();
	}

	@Override
	public Object getGroup(int groupPos) {
		// TODO Auto-generated method stub
		return listParent.get(groupPos);
	}

	@Override
	public int getGroupCount() {
		// TODO Auto-generated method stub
		return listParent.size();
	}

	@Override
	public long getGroupId(int groupPos) {
		// TODO Auto-generated method stub
		return listParent.get(groupPos).getID();
	}

	@Override
	public View getGroupView(int groupPos, boolean isExpanded, View convertView, 
			ViewGroup parent) {
		// TODO Auto-generated method stub
		ZoneViewHolder mZoneHolder;
		final Zone_c parentObj = (Zone_c)getGroup(groupPos);
		int mZoneID = parentObj.getID();
		
		if(convertView == null){
			mZoneHolder = new ZoneViewHolder();
			mZoneHolder.mZoneId = mZoneID;
			
			LayoutInflater inf = (LayoutInflater)mContext
									.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			convertView = inf.inflate(R.layout.group_list, null);
			
			
			//Random group list color
			int[] 	groupColorArr 	= mContext.getResources().getIntArray(R.array.groupColor);
			int 	groupColor 		= groupColorArr[new Random().nextInt(groupColorArr.length)];
			convertView.setBackgroundColor(groupColor);
			convertView.setTag(mZoneHolder);
		}
		else {
			mZoneHolder = (ZoneViewHolder)convertView.getTag();
			
//			if(mZoneHolder.mZoneId != mZoneID){
//				mZoneHolder = new ZoneViewHolder();
//				mZoneHolder.mZoneId = mZoneID;
//				LayoutInflater inf = (LayoutInflater)mContext
//						.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
//				convertView = inf.inflate(R.layout.group_list, null);
//			}
			convertView.setTag(mZoneHolder);
		}
//		TextView mZoneName 	= (TextView) convertView.findViewById(R.id.roomName);
		mZoneHolder.mZoneName = (TextView) convertView.findViewById(R.id.roomName);
		mZoneHolder.mZoneName.setText(parentObj.getName());
		
		return convertView;
	}

	@Override
	public boolean hasStableIds() {
		// TODO Auto-generated method stub
		return true;
	}

	@Override
	public boolean isChildSelectable(int arg0, int arg1) {
		// TODO Auto-generated method stub
		return true;
	}
	

}
