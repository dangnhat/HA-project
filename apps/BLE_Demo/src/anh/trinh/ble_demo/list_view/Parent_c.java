package anh.trinh.ble_demo.list_view;

import java.util.ArrayList;


public class Parent_c {
	
	private String 					name;
	private ArrayList<Child_c>		listChild;
	
	public Parent_c(String name, ArrayList<Child_c> listChild){
		this.name 		= name;
		this.listChild	= listChild;
	}
	
	public String  getName(){
		return name;
	}
	
	public void setName(String name){
		this.name = name;
	}
	
	public ArrayList<Child_c> getChildList(){
		return listChild;
	}
	
	public void setChildList(ArrayList<Child_c> listChild){
		this.listChild = listChild;
	}
	
}
	

