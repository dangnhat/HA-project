package anh.trinh.ble_demo;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;

public class TabsPagerAdapter extends FragmentPagerAdapter {

	public TabsPagerAdapter(FragmentManager fm) {
		super(fm);
	}

	@Override
	public Fragment getItem(int index) {
		switch(index){
		//Tab Device Control
		case 0:
			return new DeviceControlFragment();
		//Tab Scene
		case 1:
			return new ScenesFragment();
		//Tab Device Control by default
		default:
			return new DeviceControlFragment();
		}
	}

	@Override
	public int getCount() {
		//Get item count - equal number of tabs
		return 2;
	}

}
