/*
************************************************************************************************************

	Quake3:Arena Fun Name Tool
	--------------------------------

	Author: 		Jason [ZYGOTE] Brownlee
	Email:		hop_cha@hotmail.com
	Webpage:		http://planetquake.com/humandebris
	Started:		1/August/2000
	Last Updated:	1/August/2000
	File:			FunName.java
	Version:		1.0 Application

	Description:	A very simple tool for creating fun names for Quake3:Arena.
				Messing around with HTML tags in JLabels :]

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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;

public class FunName extends JFrame implements ActionListener{

	//Class Properties
	JTextField input = new JTextField(30);
	JLabel output = new JLabel();
	JLabel key = new JLabel();
	Container gui = getContentPane();
	JButton jb1 = new JButton("Process");
	JButton jb2 = new JButton("Clear");
	JPanel jp1 = new JPanel();
	JPanel jp2 = new JPanel();
	JPanel jp3 = new JPanel();

	//Constructor
	public FunName(){
		setSize(500,200);
		setTitle("Quake3:Arena Fun Names");
	  	addWindowListener(new WindowAdapter(){
	  		public void windowClosing(WindowEvent e){
	          		System.exit(0);
	      		}
	  	});
		String keystring ="<html><font size=-1>"+
			"<font color=RED>^1=RED </font>"+
			"<font color=GREEN>^2=GREEN </font>"+
			"<font color=YELLOW>^3=YELLOW </font>"+
			"<font color=BLUE>^4=BLUE </font>"+
			"<font color=#66FFFF>^5=CYAN </font>"+
			"<font color=#CC33CC>^6=MAGENTA </font>"+
			"<font color=WHITE>^7=WHITE</font>"+
			"</font></html>";
		key.setText(keystring);
		jp1.add(jb1);
		jp1.add(jb2);
		jp2.setLayout(new BorderLayout());
		jp2.add(output);
		jp2.add(key,BorderLayout.SOUTH);
		jp3.add(input);
		gui.add(jp2);
		gui.add(jp1,BorderLayout.NORTH);
		gui.add(jp3,BorderLayout.SOUTH);
		output.setHorizontalAlignment(0);
		key.setHorizontalAlignment(0);
		jp2.setBackground(Color.black);
		jb1.addActionListener(this);
		jb2.addActionListener(this);
	}

	//ActionPerformed
	public void actionPerformed(ActionEvent ae){
		Object src = ae.getSource();		

		if (src == jb1){
			String raw = input.getText();
			if (raw.length() == 0){return;}
			String temp;
			String master = "<html><font size=+3>";
			StringTokenizer tokens = new StringTokenizer(raw,"^");
			for(int i=0;tokens.hasMoreTokens();i++){
				temp = tokens.nextToken();
				if (temp.indexOf('1',0) != -1){
					master = master+"<font color=RED>"+temp.substring(1,temp.length())+"</font>";
				} else if (temp.indexOf('2',0) != -1){
					master = master+"<font color=GREEN>"+temp.substring(1,temp.length())+"</font>";
				} else if (temp.indexOf('3',0) != -1){
					master = master+"<font color=YELLOW>"+temp.substring(1,temp.length())+"</font>";
				} else if (temp.indexOf('4',0) != -1){
					master = master+"<font color=BLUE>"+temp.substring(1,temp.length())+"</font>";
				} else if (temp.indexOf('5',0) != -1){
					master = master+"<font color=#66FFFF>"+temp.substring(1,temp.length())+"</font>";
				} else if (temp.indexOf('6',0) != -1){
					master = master+"<font color=#CC33CC>"+temp.substring(1,temp.length())+"</font>";
				} else if (temp.indexOf('7',0) != -1){
					master = master+"<font color=WHITE>"+temp.substring(1,temp.length())+"</font>";
				} else{
					master = master+"<font color=WHITE>"+temp+"</font>";
				}
			}
			master = master+"</font></html>";
			output.setText(master);
		}
		if (src == jb2){
			output.setText("");
			input.setText("");
		}
	}

	//Main
	public static void main (String[] args){
		FunName fn = new FunName();
		fn.setVisible(true);
	}
}
	