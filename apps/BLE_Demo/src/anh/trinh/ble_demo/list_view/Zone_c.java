package anh.trinh.ble_demo.list_view;

import java.util.ArrayList;


public class Zone_c {
	
	private String 					name;
	private int						parentId;
	private ArrayList<Device_c>		listChild = new ArrayList<Device_c>();
	
	public Zone_c(String name, ArrayList<Device_c> listChild){
		this.name 		= name;
		this.listChild	= listChild;
	}
	
	public Zone_c() {
		// TODO Auto-generated constructor stub
	}

	public String  getName(){
		return name;
	}
	
	public void setName(int zoneID){
		this.name = "Zone " + zoneID;
		this.parentId = zoneID;
	}
	
	public int getID(){
		return this.parentId;
	}
	public ArrayList<Device_c> getChildList(){
		return listChild;
	}
	
	public void setChildList(ArrayList<Device_c> listChild){
		this.listChild.clear();
		this.listChild = listChild;
	}
	
	public void addChildListItem(Device_c child){
		this.listChild.add(child);
	}
	
	public Device_c getChildIndex(int index){
		return listChild.get(index); 
	}
	
	public int getChildCount(){
		return listChild.size();
	}
	
}
	

