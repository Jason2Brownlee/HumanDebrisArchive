/*
************************************************************************************************************

	Quake3:Arena Server Browser Tool
	--------------------------------

	Author: 		Jason [ZYGOTE] Brownlee
	Email:		hop_cha@hotmail.com
	Webpage:		http://planetquake.com/humandebris
	Started:		31/July/2000
	Last Updated:	31/July/2000
	File:			ServerBrowserTool.java
	Version:		0.9

	Description:	A simple program to query a list of servers.

	Special Thanks:	http://www.quake3arena.com/tech/ServerCommandsHowto.html

	Copyright:		Copyright (C) 2000 Jason Brownlee
				This program is free software; you can redistribute it and/or modify
  				it under the terms of the GNU General Public License as published by
   				the Free Software Foundation; either version 2 of the License, or
    				(at your option) any later version.

    				This program is distributed in the hope that it will be useful,
    				but WITHOUT ANY WARRANTY; without even the implied warranty of
    				MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    				GNU General Public License for more details.

    				You should have received a copy of the GNU General Public License
    				along with this program; if not, write to the Free Software
    				Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

************************************************************************************************************
*/

import java.net.*;
import java.io.*;	
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;
import javax.swing.table.*;
import javax.swing.border.*;
import java.lang.reflect.*;

public class ServerBrowserTool extends JFrame implements ActionListener {

	//Class Properties
	//=======================================================================================================

	//BROWSER
	JPanel browserpanel = new JPanel();
	String[] header = {"HOST","PORT","PING","MAP","CLIENTS","GAME TYPE"};
	DefaultTableModel dtm1 = new DefaultTableModel(header,0);
	JTable browser = new JTable(dtm1);
	JScrollPane jsp1 = new JScrollPane(browser);

	//CONTROLS
	JPanel controls = new JPanel();
	JButton refresh = new JButton("Refresh");
	JButton addserver = new JButton("Add Server");
	JButton deleteserver = new JButton("Delete Server");
	JButton exportlist = new JButton("Export List");
	JButton importlist = new JButton("Import List");
	JButton clear = new JButton("Clear");

	//LOG
	JTextField log = new JTextField(20);

	//CONSTANTS
	final static String GETINFO = "getinfo\0";
	final static String error1 = "# Cannot find/access server database!";
	final static String error2 = "# Server contains no servers to import!";
	final static String error3 = "# Table contains no servers to export!";
	final static String error4 = "# Cannot export vector to file!";
	final static String error5 = "# Nothing Entered!";
	final static String error6 = "# You must enter a SERVER and a PORT sperated by a :";
	final static String error7 = "# You must select a row to delete!";
	
	//OTHER
	Container gui = getContentPane();
	TableColumn browsercolumn;
	JTableHeader browserheader;
	long PingTime=0;		//Time it takes to send and receive a datagram through the socket
	int Timeout = 3000;	//How long the program will wait for a response
	JProgressBar progressBar = new JProgressBar();

	// Constructor
	//=======================================================================================================
	public ServerBrowserTool() {
		setBounds(0,0,800,570);		//perfect fit in a 800x600 window
		setTitle("Quake3:Arena Server Browser Tool");
 	  	addWindowListener(new WindowAdapter(){
	  		public void windowClosing(WindowEvent e){
	          		System.exit(0);
	      		}
	  	});
		//BROWSER
		browserpanel.setLayout(new BorderLayout());
		browserpanel.add(progressBar,BorderLayout.NORTH);
		browserpanel.setBorder(BorderFactory.createTitledBorder("Browser"));	
		browserpanel.add(jsp1);

		//CONTROLS
		controls.setBorder(BorderFactory.createTitledBorder("Controls"));	
		controls.add(refresh);
		controls.add(addserver);
		controls.add(deleteserver);
		controls.add(exportlist);
		controls.add(importlist);
		controls.add(clear);

		//Content Pane
		gui.add(browserpanel);
		gui.add(controls,BorderLayout.NORTH);
		gui.add(log,BorderLayout.SOUTH);

		//LOG
		log.setBackground(Color.black);
		log.setForeground(Color.white);

		//ACTION LISTENERS	
		refresh.addActionListener(this);
		addserver.addActionListener(this);
		deleteserver.addActionListener(this);
		exportlist.addActionListener(this);
		importlist.addActionListener(this);
		clear.addActionListener(this);

		//TABLE STUFF
		browsercolumn = browser.getColumnModel().getColumn(0);
		browsercolumn.setPreferredWidth(Math.max(300, 300));
		browsercolumn = browser.getColumnModel().getColumn(3);
		browsercolumn.setPreferredWidth(Math.max(100, 100));
		browsercolumn = browser.getColumnModel().getColumn(5);
		browsercolumn.setPreferredWidth(Math.max(150, 150));
		browserheader = browser.getTableHeader();
		browserheader.setReorderingAllowed(false);
		browserheader.setResizingAllowed(false);

		//ProgressBar
		progressBar.setStringPainted(true);

	} // Constructor

	//ActionListener
	//=======================================================================================================
	public void actionPerformed(ActionEvent ae){
		Object src = ae.getSource();		
		if (src == refresh){			
			new RefreshThread().start();
		}
		if (src == addserver){
			String jop, Host, Port;
			jop = JOptionPane.showInputDialog("Server:Port");
			if (jop == null){
				log.setText(error5);
				return;
			}
			int cTest = jop.indexOf(':');
			if (cTest == -1){
				log.setText(error6);
				return;
			}
			Host = jop.substring(0,cTest);
			Port = jop.substring(cTest+1,jop.length());
			String [] newRow = {Host,Port,"?","?","?","?"};
			dtm1.addRow(newRow);
		}
		if (src == deleteserver){
			int rowToDelete = browser.getSelectedRow();
			if (rowToDelete < 0){
				log.setText(error7);
				return;
			}
			dtm1.removeRow(rowToDelete);
		}
		if (src == exportlist){
			ExportBrowserTable();
		}
		if (src == importlist){
			BuildBrowserTable();
		}
		if (src == clear){
			while (dtm1.getRowCount() != 0) {	//Clear Browser
				dtm1.removeRow(0);
			}
		}
	}

	//Build Table From Vector
	//=======================================================================================================
	public void BuildBrowserTable() {	
		String VectorString,Host,Port;
		Vector VectorFromFile = new Vector();
		try {
			VectorFromFile = ImportServerList(VectorFromFile);			
		} catch (IOException IOE){
			log.setText(error1);
			return;	
		}
		int vSize = VectorFromFile.size();
		if (vSize==0) {
			log.setText(error2);
			return;
		}
		dtm1.setNumRows(vSize);
		for (int i=0; i<vSize;i++){
			VectorString = VectorFromFile.elementAt(i).toString();
			StringTokenizer tokens = new StringTokenizer(VectorString,":");
			Host = tokens.nextToken();
			Port = tokens.nextToken();
			dtm1.setValueAt(Host,i,0);
			dtm1.setValueAt(Port,i,1);
		}
	}

	//Build Vector from file
	//=======================================================================================================
	public Vector ImportServerList(Vector VectorFromFile) throws IOException{
		FileReader fr = new FileReader("./servers.q3");
		BufferedReader br = new BufferedReader(fr);
		String s = br.readLine();
		while (s != null) {
			VectorFromFile.addElement(s);
			s = br.readLine();
		}
		return VectorFromFile;
	}

	//Build Vector From Browser Table
	//=======================================================================================================
	public void ExportBrowserTable() {
		Vector vc = new Vector();
		String Host,Port;
		int rowCount = dtm1.getRowCount();
		for (int i=0; i<rowCount;i++){
			Host = dtm1.getValueAt(i,0).toString();
			Port = dtm1.getValueAt(i,1).toString();
			vc.addElement(Host+":"+Port);
		}
		if (vc.size()==0) {
			log.setText(error3);
			return;
		}
		//Pass vector to file
		try {
			ExportServerList(vc);			
		} catch (IOException IOE){
			log.setText(error4);
		}
	}

	//Build File From Browser Vector
	//=======================================================================================================
	public void ExportServerList(Vector vc) throws IOException{
		String s;
		FileWriter fw = new FileWriter("./servers.q3");
		BufferedWriter bw = new BufferedWriter(fw);
		int vSize = vc.size();
		for (int i=0; i<vSize; i++) {
			s = vc.elementAt(i).toString();
			bw.write(s);
			bw.newLine();
		}
		bw.close();
	}

	//Build, Send and Receive Datagram (Return a String)
	//=======================================================================================================
	private String ServerCommunication(InetAddress Host, int Port, String serverquery){
		DatagramSocket serversocket = null;			// Datagram Socket
		DatagramPacket mypacket = null;			// Datagram Packet	
		byte[] packetBuffer = new byte[65507];		// Packet Buffer
		long starttime = 0;					// Start Time
		StringBuffer sbuffer = new StringBuffer();	// for building the data string
		
		//Creating and 'Out Of Band' Packet (OOB)
 		String out = "xxxx" + serverquery;		
		byte [] buff = out.getBytes();		
      	buff[0] = (byte)0xff; 
      	buff[1] = (byte)0xff;
      	buff[2] = (byte)0xff;
      	buff[3] = (byte)0xff;

		//Create The Socket
		try{
			serversocket = new DatagramSocket();
			serversocket.setSoTimeout(Timeout);
		}catch(IOException e){return "";}

		//Create Packet
		mypacket = new DatagramPacket(buff, buff.length, Host, Port);

		//Send Packet through Socket
		try{
			serversocket.send(mypacket);
			starttime = System.currentTimeMillis();
		}catch(IOException e){return "";}
		
		//Create a PacketBuffer for Receiving
		mypacket = new DatagramPacket(packetBuffer, packetBuffer.length);
	
		//Receive Packet
		try{
			serversocket.receive(mypacket);
		}catch(IOException e){return "";}

		//Record How long it took to (Send-Receive) the Packet
		PingTime = (System.currentTimeMillis() - starttime);		

		//Build a StringBuffer From the returned data
		int packetlength = mypacket.getLength();					
		for (int i=0; i < packetlength; i++) {
			sbuffer.append((char)packetBuffer[i]);			
		}

		//Convert the StringBuffer to a String and Return it
		String output = sbuffer.toString();	
		return output;
	}

	//Build The Row Array From the ServerString
	//=======================================================================================================
	private String[] GetStringToArray(String serverdata){
		Long ping = new Long(PingTime);
		String mapname = "???";
		String clients = "???";
		String gametype = "???";
		String maxclients ="???";
		int gtype = 0;

		StringTokenizer tokens = new StringTokenizer(serverdata,"\\");
		for (int t=0; tokens.hasMoreTokens(); t++) {	
     			String str1 = tokens.nextToken();
			if (str1.compareTo("mapname") == 0) {
				str1 = tokens.nextToken();
				mapname = str1;
			}
			if (str1.compareTo("clients") == 0) {
				str1 = tokens.nextToken();
				clients = str1;
			}
			if (str1.compareTo("gametype") == 0) {
				str1 = tokens.nextToken();
				gametype = str1;
			}
			if (str1.compareTo("sv_maxclients") == 0) {
				str1 = tokens.nextToken();
				maxclients = str1;
			}
		}
		try {
			gtype = Integer.parseInt(gametype);
		} catch (NumberFormatException e){
			String[] arrayfortable = {ping.toString(),mapname,clients+"\\"+maxclients,gametype};
			return arrayfortable;
		}
		switch (gtype){
		case 0: 	gametype = "Free For All";
				break;
		case 1: 	gametype = "Tournament";
				break;
		case 2: 	gametype = "Single Player";
				break;
		case 3: 	gametype = "Team Death Match";
				break;
		case 4: 	gametype = "Capture The Flag";
				break;
		default: 	gametype = "???";
				break;
		}
		String[] arrayfortable = {ping.toString(),mapname,clients+"\\"+maxclients,gametype};
		return arrayfortable;
	}

	//Main
	//=======================================================================================================
	public static void main (String[] args) {
		ServerBrowserTool sbt = new ServerBrowserTool();
		sbt.setVisible(true);
	}

	//Refresh Thread
	//=======================================================================================================
	class RefreshThread extends Thread {
		String serveroutput = "";			
		InetAddress Host = null;
		int Port = 0;

		public void RefreshThread() {}	//No Constructing

		//RUN
		//=================================================================================================
		public void run(){
			//Disable All Buttons
			refresh.setEnabled(false);
			addserver.setEnabled(false);
			deleteserver.setEnabled(false);
			exportlist.setEnabled(false);
			importlist.setEnabled(false);
			clear.setEnabled(false);

			int TotalRows = dtm1.getRowCount();
			progressBar.setMinimum(0);
			progressBar.setMaximum(TotalRows);

			for(int i=0;i<TotalRows;i++){
				try{
					Host = InetAddress.getByName(dtm1.getValueAt(i,0).toString());
					browser.setValueAt(Host.getHostName(),i,0);
				}catch(UnknownHostException e){Host = null;}		
				try {
					Port = Integer.parseInt(dtm1.getValueAt(i,1).toString());
				} catch (NumberFormatException e){Port = 0;}
				if ((Host!=null)&&(Port>0)&&(Port<=60000)){					
					serveroutput = ServerCommunication(Host,Port,GETINFO);					
				} else {
					serveroutput = "";
				}
				if(serveroutput.length() != 0){
					String[] fullarray = GetStringToArray(serveroutput);
					browser.setValueAt(fullarray[0],i,2);
					browser.setValueAt(fullarray[1],i,3);
					browser.setValueAt(fullarray[2],i,4);
					browser.setValueAt(fullarray[3],i,5);
				}else{
					//Do some cell clearing
					browser.setValueAt("",i,2);
					browser.setValueAt("",i,3);
					browser.setValueAt("",i,4);
					browser.setValueAt("",i,5);
				}
				progressBar.setValue(i);
			}
			progressBar.setValue(0);
			//Enable All Buttons
			refresh.setEnabled(true);
			addserver.setEnabled(true);
			deleteserver.setEnabled(true);
			exportlist.setEnabled(true);
			importlist.setEnabled(true);
			clear.setEnabled(true);
		}
	}
}
//EOF