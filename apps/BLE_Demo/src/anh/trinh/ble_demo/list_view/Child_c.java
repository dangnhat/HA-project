package anh.trinh.ble_demo.list_view;

public class Child_c {
	
	private String	name;
	private int	val;
	
	public Child_c(String name, int val){
		this.name = name;
		this.val  = val;
	}
	
	public String getName(){
		return name;
	}
	
	public int getVal(){
		return val;
	}
	
	public void setName(String name){
		this.name = name;
	}
	
	public void setVal(int val){
		this.val = val;
	}
}
