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
import android.widget.BaseExpandableListAdapter;
import android.widget.TextView;
import anh.trinh.ble_demo.R;
import anh.trinh.ble_demo.R.color;


public class ExpandableListViewAdapter extends BaseExpandableListAdapter {
	
	
	private Activity 				mContext;
	private ArrayList<Parent_c>		listParent;
	private ArrayList<Child_c> 		listChild;
	/**
	 * Constructor 
	 */
	public ExpandableListViewAdapter(	Activity mContext, 
										ArrayList<Parent_c> listParent,
										ArrayList<Child_c>  listChild	)
	{
		this.mContext	= mContext;
		this.listParent	= listParent;
		this.listChild	= listChild;
	}

	@Override
	public Object getChild(int groupPos, int childPos) {
		// TODO Auto-generated method stub
		return listChild.get(childPos);
	}

	@Override
	public long getChildId(int groupPos, int childPos) {
		// TODO Auto-generated method stub
		return childPos;
	}

	@Override
	public View getChildView(int groupPos, int childPos, boolean isLastChild, View convertView,
			ViewGroup parent) {
		// TODO Auto-generated method stub
		final Child_c childObj	= (Child_c) getChild(groupPos, childPos);
		if(convertView == null){
			LayoutInflater inf 	= 	(LayoutInflater) mContext
										.getSystemService(mContext.LAYOUT_INFLATER_SERVICE);
			convertView			=	inf.inflate(R.layout.device_list, null);
		}
		
		TextView devName 	= (TextView) convertView.findViewById(R.id.devName);
		TextView devVal 	= (TextView) convertView.findViewById(R.id.devVal);
 		devName.setText(childObj.getName());
 		devVal.setText(Integer.toString(childObj.getVal()));
		
		return convertView;
	}

	@Override
	public int getChildrenCount(int groupPos) {
		// TODO Auto-generated method stub
		return listParent.get(groupPos).getChildList().size();
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
		return groupPos;
	}

	@Override
	public View getGroupView(int groupPos, boolean isExpanded, View convertView, 
			ViewGroup parent) {
		// TODO Auto-generated method stub
		final Parent_c parentObj = (Parent_c)getGroup(groupPos);
		
		if(convertView == null){
			LayoutInflater inf = (LayoutInflater)mContext
									.getSystemService(mContext.LAYOUT_INFLATER_SERVICE);
			convertView = inf.inflate(R.layout.group_list, null);
			
			//Random group list color
			int[] 	groupColorArr 	= mContext.getResources().getIntArray(R.array.groupColor);
			int 	groupColor 		= groupColorArr[new Random().nextInt(groupColorArr.length)];
			convertView.setBackgroundColor(groupColor);
			Log.i("ConvertViewLog", "WTF?");
		}
		
		TextView roomName 	= (TextView) convertView.findViewById(R.id.roomName);
		roomName.setText(parentObj.getName());
		
		return convertView;
	}

	@Override
	public boolean hasStableIds() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isChildSelectable(int arg0, int arg1) {
		// TODO Auto-generated method stub
		return false;
	}

}
