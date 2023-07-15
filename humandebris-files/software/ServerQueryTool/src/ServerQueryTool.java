/*
-------------------------------------------------------------------------------------
Quake3:Arena Server Query Tool
-------------------------------------------------------------------------------------
Author: 		Jason [ZYGOTE] Brownlee
Email:			hop_cha@hotmail.com
Webpage:		http://planetquake.com/humandebris
Date:			31/July/2000
Revised:		31/November/2000
File:			ServerQueryTool.java

Description:	An application to query Quake III Arena servers.

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
-------------------------------------------------------------------------------------
*/

//AWT EVENTS
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.WindowListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowAdapter;
//AWT
import java.awt.Container;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.Image;
import java.awt.Graphics;
import java.awt.Color;
//SWING
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.JTable;
import javax.swing.JScrollPane;
//TABLE
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableColumn;
import javax.swing.table.JTableHeader;
//BORDER
import javax.swing.BorderFactory;


/*
Server Query Tool
*/
public class ServerQueryTool extends JFrame implements ActionListener
{
	//Class Properites
	private JButton ping_button = new JButton("Ping");
	private JButton status_button = new JButton("Status");
	private JButton info_button = new JButton("Info");
	private JButton clear_button = new JButton("Clear");
	private JTextField host_field = new JTextField("localhost", 10);
	private JTextField port_field = new JTextField("27960", 10);
	private JTextField host_name = new JTextField(10);
	private JTextField host_ip = new JTextField(10);
	private JTextField host_ping = new JTextField(10);
	private JLevelshotPanel levelshot = new JLevelshotPanel(120,120);
	private QuakeServerCommunication comm = new QuakeServerCommunication(3000, false);
	private Q3Utils q3utils = new Q3Utils(false);
	private CustomTableModel status_model = new CustomTableModel();
	private CustomTableModel info_model = new CustomTableModel();
	private CustomTableModel players_model = new CustomTableModel();
	private JTable ping_table;
	private JTable status_table = new JTable(status_model);
	private JTable info_table = new JTable(info_model);
	private JTable players_table = new JTable(players_model);
	private JPanel levelshot_panel = new JPanel();

	/*
	Constructor
	*/
	public ServerQueryTool()
	{
		super("QIIIA Server Query Tool - Human Debris 2000");
		BuildGUI(getContentPane());
      	setBounds(0,0,800,570);
 	  	addWindowListener(new WindowAdapter(){
	  		public void windowClosing(WindowEvent e){System.exit(0);}
	  	});
	  	setVisible(true);
	  	ClearGui();
	}

	/*
	BuildGUI
	*/
	public void BuildGUI(Container gui)
	{
		TableColumn table_column;
		JTableHeader table_header;
		CustomTableModel ping_model = new CustomTableModel();
		ping_table = new JTable(ping_model);

		//North
		JPanel north_panel = new JPanel(new BorderLayout());
		JPanel north_west_panel = new JPanel(new GridLayout(2,1));
		JPanel north_center_panel = new JPanel(new GridLayout(2,1));
		north_west_panel.add(new JLabel(" HOST: "));
		north_west_panel.add(new JLabel(" PORT: "));
		north_center_panel.add(host_field);
		north_center_panel.add(port_field);
		north_panel.add(north_west_panel, BorderLayout.WEST);
		north_panel.add(north_center_panel, BorderLayout.CENTER);
		north_panel.setBorder(BorderFactory.createTitledBorder("Input"));

		//Center
		JPanel center_panel = new JPanel(new BorderLayout());
		JPanel center_north = new JPanel(new GridLayout(1,1));
		JPanel center_center = new JPanel(new GridLayout(2,2));
		center_panel.add(center_north, BorderLayout.NORTH);
		center_panel.add(center_center, BorderLayout.CENTER);
		center_panel.setBorder(BorderFactory.createTitledBorder("Output"));

		//Ping Table
		ping_model.setColumnIdentifiers(new String[] {"Protocol","Hostname","Map Name","Clients","Gametype","Pure"});
		ping_model.setNumRows(1);
		table_header = ping_table.getTableHeader();
		table_header.setReorderingAllowed(false);
		table_header.setResizingAllowed(false);
		ping_table.setCellSelectionEnabled(false);
		ping_table.setRowSelectionAllowed(false);
		ping_table.setColumnSelectionAllowed(false);
		ping_table.setPreferredScrollableViewportSize(new Dimension(1,20));
		center_north.add(new JScrollPane(ping_table));
		table_column = ping_table.getColumnModel().getColumn(0);
		table_column.setMaxWidth(60);
		table_column.setPreferredWidth(60);
		table_column = ping_table.getColumnModel().getColumn(2);
		table_column.setMaxWidth(150);
		table_column.setPreferredWidth(150);
		table_column = ping_table.getColumnModel().getColumn(3);
		table_column.setMaxWidth(70);
		table_column.setPreferredWidth(70);
		table_column = ping_table.getColumnModel().getColumn(5);
		table_column.setMaxWidth(40);
		table_column.setPreferredWidth(40);

		//Status Table
		status_model.setColumnIdentifiers(new String[] {"Variable","Value"});
		status_model.setNumRows(10);
		table_header = status_table.getTableHeader();
		table_header.setReorderingAllowed(false);
		table_header.setResizingAllowed(false);
		status_table.setCellSelectionEnabled(false);
		status_table.setRowSelectionAllowed(false);
		status_table.setColumnSelectionAllowed(false);
		center_center.add(new JScrollPane(status_table));
		table_column = status_table.getColumnModel().getColumn(0);
		table_column.setMaxWidth(120);
		table_column.setPreferredWidth(120);

		//Players Table
		players_model.setColumnIdentifiers(new String[] {"Score","Ping","Player Name"});
		players_model.setNumRows(10);
		table_header = players_table.getTableHeader();
		table_header.setReorderingAllowed(false);
		table_header.setResizingAllowed(false);
		players_table.setCellSelectionEnabled(false);
		players_table.setRowSelectionAllowed(false);
		players_table.setColumnSelectionAllowed(false);
		center_center.add(new JScrollPane(players_table));
		table_column = players_table.getColumnModel().getColumn(0);
		table_column.setMaxWidth(40);
		table_column.setPreferredWidth(40);
		table_column = players_table.getColumnModel().getColumn(1);
		table_column.setMaxWidth(40);
		table_column.setPreferredWidth(40);

		//Info Table
		info_model.setColumnIdentifiers(new String[] {"Variable","Value"});
		info_model.setNumRows(10);
		table_header = info_table.getTableHeader();
		table_header.setReorderingAllowed(false);
		table_header.setResizingAllowed(false);
		info_table.setCellSelectionEnabled(false);
		info_table.setRowSelectionAllowed(false);
		info_table.setColumnSelectionAllowed(false);
		center_center.add(new JScrollPane(info_table));
		table_column = info_table.getColumnModel().getColumn(0);
		table_column.setMaxWidth(120);
		table_column.setPreferredWidth(120);

		//Levelshot Label
		center_center.add(levelshot_panel);
		levelshot_panel.add(levelshot);

		//South
		JPanel south_panel = new JPanel(new BorderLayout());
		JPanel south_north = new JPanel(new BorderLayout());
		JPanel south_center = new JPanel();
		JPanel south_center_west = new JPanel(new GridLayout(3,1));
		JPanel south_center_center = new JPanel(new GridLayout(3,1));
		south_center_west.add(new JLabel(" Hostname Address: "));
		south_center_west.add(new JLabel(" Host IP Address: "));
		south_center_west.add(new JLabel(" Host Ping: "));
		south_center_center.add(host_name);
		south_center_center.add(host_ip);
		south_center_center.add(host_ping);
		south_north.add(south_center_west,BorderLayout.WEST);
		south_north.add(south_center_center,BorderLayout.CENTER);
		south_north.setBorder(BorderFactory.createTitledBorder("Host"));
		host_name.setEditable(false);
		host_ip.setEditable(false);
		host_ping.setEditable(false);
		south_center.setBorder(BorderFactory.createTitledBorder("Controls"));
		south_center.add(ping_button);
		south_center.add(status_button);
		south_center.add(info_button);
		south_center.add(clear_button);
		ping_button.addActionListener(this);
		status_button.addActionListener(this);
		info_button.addActionListener(this);
		clear_button.addActionListener(this);
		south_panel.add(south_north,BorderLayout.NORTH);
		south_panel.add(south_center,BorderLayout.CENTER);

		//GUI
		gui.add(north_panel, BorderLayout.NORTH);
		gui.add(south_panel, BorderLayout.SOUTH);
		gui.add(center_panel, BorderLayout.CENTER);
	}


	/*
	PingServer
	*/
	public void PingServer()
	{
		String raw;
		String formatted[];
		int i;

		if((raw=comm.QueryServer(host_field.getText(), port_field.getText(), comm.GETINFO))!=null)
		{
			if((formatted=q3utils.BuildPingArray(raw))!=null)
			{
				for(i=0; i<formatted.length; i++)
				{
					ping_table.setValueAt(formatted[i], 0, i);
				}
				UpdateGui(formatted[2]);
			}
		}
	}

	/*
	InfoServer
	*/
	public void InfoServer()
	{
		String raw;
		String formatted[][];
		int i;
		String map=null;

		if((raw=comm.QueryServer(host_field.getText(), port_field.getText(), comm.GETINFO))!=null)
		{
			if((formatted=q3utils.BuildInfoArray(raw))!=null)
			{
				info_model.setNumRows(formatted.length);
				for(i=0; i<formatted.length; i++)
				{
					info_table.setValueAt(formatted[i][0], i, 0);
					info_table.setValueAt(formatted[i][1], i, 1);
					if(formatted[i][0].equals("mapname"))
					{
						map = formatted[i][1];
					}
				}
				UpdateGui(map);
			}
		}
	}

	/*
	StatusServer
	*/
	public void StatusServer()
	{
		String raw;
		String formatted_status[][];
		String formatted_players[][];
		int i;
		String map=null;
		String temp[];

		if((raw=comm.QueryServer(host_field.getText(), port_field.getText(), comm.GETSTATUS))!=null)
		{
			if((temp=q3utils.SeperateStatusArray(raw))!=null)
			{
				if((formatted_status=q3utils.BuildStatusArray(temp[0]))!=null)
				{
					status_model.setNumRows(formatted_status.length);
					for(i=0; i<formatted_status.length; i++)
					{
						status_table.setValueAt(formatted_status[i][0], i, 0);
						status_table.setValueAt(formatted_status[i][1], i, 1);
						if(formatted_status[i][0].equals("mapname"))
						{
							map = formatted_status[i][1];
						}
					}
				}
				if((formatted_players=q3utils.BuildPlayersArray(temp[1]))!=null)
				{
					players_model.setNumRows(formatted_players.length);
					for(i=0; i<formatted_players.length; i++)
					{
						players_table.setValueAt(formatted_players[i][0], i, 0);
						players_table.setValueAt(formatted_players[i][1], i, 1);
						players_table.setValueAt(formatted_players[i][2], i, 2);
					}
				}
				UpdateGui(map);
			}
		}
	}

	/*
	ClearGui
	*/
	public void ClearGui()
	{
		host_name.setText("");
		host_ip.setText("");
		host_ping.setText("");
		levelshot.displayImage("unknownmap.gif");
		//clear ping
		for(int i=0; i<6; i++)
		{
			ping_table.setValueAt("", 0, i);
		}
		status_model.setNumRows(0);
		players_model.setNumRows(0);
		info_model.setNumRows(0);
	}

	/*
	UpdateGui
	*/
	public void UpdateGui(String map)
	{
		host_name.setText(comm.GetHostName());
		host_ip.setText(comm.GetIPAddress());
		host_ping.setText(Long.toString(comm.GetPing()));
		levelshot.displayImage("levelshots/"+map+".jpg");
	}

	/*
	Action Performed
	*/
	public void actionPerformed(ActionEvent AE)
	{
		Object src = AE.getSource();
		if(src==ping_button)
		{
			PingServer();
		}else if(src==status_button){
			StatusServer();
		}else if(src==info_button){
			InfoServer();
		}else if(src==clear_button){
			ClearGui();
		}
	}

	/*
	Main Method
	*/
	public static void main(String[] args)
	{
		ServerQueryTool tool = new ServerQueryTool();
	}
}


/*
Custom Table Model
To stop cells from being editable.
*/
class CustomTableModel extends DefaultTableModel
{
	public CustomTableModel()
	{
		super();
	}
	public boolean isCellEditable(int row, int column)
	{
		return false;
	}
}


/*
JLevelshot Panel
*/
class JLevelshotPanel extends JPanel
{
	private Image levelshot=null;
	private Toolkit tk = Toolkit.getDefaultToolkit();
	private Dimension size;

	/*
	Constructor
	*/
	public JLevelshotPanel(int x, int y)
	{
		this.setBackground(Color.black);
		if(x<10 || y<10){
			size = new Dimension(100,100);
		}else{
			size = new Dimension(x,y);
		}
		this.setPreferredSize(size);
	}

	/*
	Called by a Component.reapint()
	*/
	public void paintComponent(Graphics g)
	{
		super.paintComponent(g);
		if(levelshot!=null){
			try{
				g.drawImage(levelshot, 0, 0, size.width, size.height, this);
			}catch(Exception e){
				levelshot=null;
				System.out.println("DEGUG: "+e.toString());
			}
		}else{
			try{
				levelshot = tk.getImage("unknownmap.gif");
				g.drawImage(levelshot, 0, 0, size.width, size.height, this);
			}catch(Exception e){levelshot=null;}
		}
	}

	/*
	Get the image
	*/
	public void displayImage(String s)
	{
		if(s==null)
		{
			levelshot=null;
		}else{
			try{
				levelshot = tk.getImage(s);
				this.repaint();
			}catch(Exception e){
				levelshot=null;
				System.out.println("DEGUG: "+e.toString());
			}
		}
	}
}

