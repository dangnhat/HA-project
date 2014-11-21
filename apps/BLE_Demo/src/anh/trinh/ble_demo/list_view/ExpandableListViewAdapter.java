package anh.trinh.ble_demo.list_view;

import java.util.ArrayList;
import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseExpandableListAdapter;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.ToggleButton;
import anh.trinh.ble_demo.ColorPickerDialog;
import anh.trinh.ble_demo.ColorPickerDialog.OnColorChangedListener;
//import android.widget.
import anh.trinh.ble_demo.HomeActivity;
import anh.trinh.ble_demo.R;
import anh.trinh.ble_demo.data.BluetoothMessage;
import anh.trinh.ble_demo.data.CommandID;
import anh.trinh.ble_demo.data.DataConversion;
import anh.trinh.ble_demo.data.DeviceTypeDef;


public class ExpandableListViewAdapter extends BaseExpandableListAdapter {
	
	
	private HomeActivity 				mContext;
	private ArrayList<Zone_c>		listParent;
	private class DeviceHolder{
		public int 			mHolderType;
		public TextView 	mDevName;
		public ImageView 	mDevImg;
		public TextView		mDevVal;
		public SeekBar		mSeekBar;
		public ToggleButton mBtnOnOff;
	}
	
	/**
	 * Constructor 
	 */
	public ExpandableListViewAdapter(	HomeActivity mContext, 
										ArrayList<Zone_c> listParent)
	{
		this.mContext	= mContext;
		this.listParent	= listParent;
	}
	
	/**
	 * Update list data
	 */
	public void updateData(ArrayList<Zone_c> listParent){
		this.listParent = listParent;
	}
	
	@Override
	public View getGroupView(int groupPos, boolean isExpanded, View convertView, 
			ViewGroup parent) {
		// TODO Auto-generated method stub
		final Zone_c parentObj = (Zone_c)getGroup(groupPos);
		
		if(convertView == null){
			LayoutInflater inf = (LayoutInflater)mContext
									.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			convertView = inf.inflate(R.layout.group_list, null);
			
		}
		TextView mZoneName = (TextView) convertView.findViewById(R.id.roomName);
		mZoneName.setText(parentObj.getName());
		
		return convertView;
	}

	@Override
	public View getChildView(int groupPos, int childPos, boolean isLastChild, View convertView,
			ViewGroup parent) {
		// TODO Auto-generated method stub
		
		final Device_c childObj	= (Device_c) getChild(groupPos, childPos);
		int devType = childObj.getID();
		short devVal = (short) childObj.getVal();
		final DeviceHolder mDevViewHolder;
		
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
			case DeviceTypeDef.EVENT_SENSOR:
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
			case DeviceTypeDef.LINEAR_SENSOR:
				convertView = inf.inflate(R.layout.sensor_dev, null);
				mDevViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
				mDevViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
				mDevViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
				break;
			case DeviceTypeDef.SERVO_SG90:
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
				DeviceHolder mViewHolder = new DeviceHolder();
				mViewHolder.mHolderType  = devType;
				
				// assign layout match with type of device
				switch (devType) {
				
				case DeviceTypeDef.BUTTON:
					convertView = inf.inflate(R.layout.switch_item, null);
					mViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mViewHolder.mBtnOnOff = (ToggleButton)convertView.findViewById(R.id.toggleVal);
					break;
				case DeviceTypeDef.DIMMER:
					convertView = inf.inflate(R.layout.bulb, null);
					mViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mViewHolder.mSeekBar = (SeekBar)convertView.findViewById(R.id.dimBar);
					mViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.EVENT_SENSOR:
					convertView = inf.inflate(R.layout.sensor_dev, null);
					mViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.LEVEL_BULB:
					convertView = inf.inflate(R.layout.bulb, null);
					mViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mViewHolder.mSeekBar = (SeekBar)convertView.findViewById(R.id.dimBar);
					mViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.LINEAR_SENSOR:
					convertView = inf.inflate(R.layout.sensor_dev, null);
					mViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.SERVO_SG90:
					convertView = inf.inflate(R.layout.bulb, null);
					mViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mViewHolder.mSeekBar = (SeekBar)convertView.findViewById(R.id.dimBar);
					mViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.ON_OFF_BULB:
					convertView = inf.inflate(R.layout.switch_item, null);
					mViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mViewHolder.mBtnOnOff = (ToggleButton)convertView.findViewById(R.id.toggleVal);	
					break;
				case DeviceTypeDef.RGB_LED:
					convertView = inf.inflate(R.layout.sensor_dev, null);
					mViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				case DeviceTypeDef.SWITCH:
					convertView = inf.inflate(R.layout.switch_item, null);
					mViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mViewHolder.mBtnOnOff = (ToggleButton)convertView.findViewById(R.id.toggleVal);
					break;
				default:
					convertView = inf.inflate(R.layout.device_list, null);
					mViewHolder.mDevName = (TextView)convertView.findViewById(R.id.devName);
					mViewHolder.mDevImg = (ImageView)convertView.findViewById(R.id.devIcon);
					mViewHolder.mDevVal = (TextView)convertView.findViewById(R.id.devVal);
					break;
				};
				
				convertView.setTag(mViewHolder);
			}
		}
		
		switch (mDevViewHolder.mHolderType) {
		case DeviceTypeDef.BUTTON:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.button_push);
			mDevViewHolder.mBtnOnOff.setClickable(false);
			if(devVal == 0){
				mDevViewHolder.mBtnOnOff.setChecked(false);
			}  
			else{
				mDevViewHolder.mBtnOnOff.setChecked(true);
			}
			break;
		case DeviceTypeDef.DIMMER:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.dimmer);
			mDevViewHolder.mSeekBar.setProgress(devVal);
			mDevViewHolder.mDevVal.setText(Short.toString(devVal));
			mDevViewHolder.mSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
				int start, stop, delta;
				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {
					// TODO Auto-generated method stub
					stop = seekBar.getProgress();
					Log.i("Dimmer", "Stop" + Integer.toString(stop));
					delta = Math.abs(stop - start);
					Log.i("Dimmer", "delta" + Integer.toString(delta));
				}
				
				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {
					// TODO Auto-generated method stub
					start = seekBar.getProgress();
					Log.i("Dimmer", "Start" + Integer.toString(start));
				}
				
				@Override
				public void onProgressChanged(SeekBar seekBar, int progress,
						boolean fromUser) {
					// TODO Auto-generated method stub
					mDevViewHolder.mDevVal.setText(Integer.toString(progress));
				}
			});
			break;	
		case DeviceTypeDef.EVENT_SENSOR:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.gas_sensor);
			mDevViewHolder.mDevVal.setText(Short.toString(devVal));
			break;
		case DeviceTypeDef.LEVEL_BULB:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.bulb);
			if(devVal == 0){
				mDevViewHolder.mBtnOnOff.setChecked(false);
			}  
			else{
				mDevViewHolder.mBtnOnOff.setChecked(true);
			}
			break;
		case DeviceTypeDef.LINEAR_SENSOR:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.light_sensor);
			mDevViewHolder.mDevVal.setText(Short.toString(devVal) + "lux");
			break;
		case DeviceTypeDef.SERVO_SG90:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.motor);
			mDevViewHolder.mDevVal.setText(Short.toString(devVal));
			mDevViewHolder.mSeekBar.setMax(180);
			mDevViewHolder.mSeekBar.setProgress(devVal);
			mDevViewHolder.mSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
				
				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {
					// TODO Auto-generated method stub
					
				}
				
				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {
					// TODO Auto-generated method stub
					
				}
				
				@Override
				public void onProgressChanged(SeekBar seekBar, int progress,
						boolean fromUser) {
					// TODO Auto-generated method stub
					mDevViewHolder.mDevVal.setText(Integer.toString(progress));
					
				}
			});
			break;
		case DeviceTypeDef.ON_OFF_BULB:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.bulb);
			if(devVal == 0){
				mDevViewHolder.mBtnOnOff.setChecked(false);
			}  
			else{
				mDevViewHolder.mBtnOnOff.setChecked(true);
			}
			
			
		break;
		case DeviceTypeDef.RGB_LED:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.rgb_led);
			mDevViewHolder.mDevVal.setText("0x"+Integer.toHexString(devVal & 0xFFFF));
			mDevViewHolder.mDevVal.setOnClickListener(new OnClickListener() {
				
				@Override
				public void onClick(View arg0) {
					// TODO Auto-generated method stub
					new ColorPickerDialog(mContext, new OnColorChangedListener() {
						
						@Override
						public void colorChanged(int color) {
							// TODO Auto-generated method stub
							
						}
					}, new Color().CYAN).show();
				}
			});
			break;
		case DeviceTypeDef.SWITCH:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.switch_btn);
			if(devVal == 0){
				mDevViewHolder.mBtnOnOff.setChecked(false);
			}  
			else{
				mDevViewHolder.mBtnOnOff.setChecked(true);
			}
			break;
		default:
			mDevViewHolder.mDevImg.setImageResource(R.drawable.inknon);
			break;
		}
		mDevViewHolder.mDevName.setText(childObj.getName());
		return convertView;
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
	public boolean hasStableIds() {
		// TODO Auto-generated method stub
		return true;
	}

	@Override
	public boolean isChildSelectable(int arg0, int arg1) {
		// TODO Auto-generated method stub
		return true;
	}
	
	@Override
	public void notifyDataSetChanged() {
		// TODO Auto-generated method stub
		super.notifyDataSetChanged();
//		Log.i("List Adapter", "update data");
	}
	
/***********************************************************************************************
 * Private Functions
 ***********************************************************************************************/
	
	/**
	 * 
	 * 
	 * @param devInfo
	 */
	private void sendDataUpdate(Device_c devInfo){
		BluetoothMessage bleMsg = new BluetoothMessage();
		bleMsg.setLength((byte) 6);
		bleMsg.setCmdIdH((byte) CommandID.SET);
		bleMsg.setCmdIdL((byte) CommandID.DEV_VAL);
		bleMsg.setPayload(DataConversion.devInfo2ByteArr(devInfo));
		final Message msg = mContext.mMsgHandler.obtainMessage(CommandID.DEV_VAL, (Object)bleMsg);
		mContext.mMsgHandler.sendMessage(msg);
		
	}
}
