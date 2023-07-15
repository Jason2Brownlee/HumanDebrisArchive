/*
--------------------------------------------------------------------------------
QuakeServerCommunication
For querying Quake III Arena Servers.
--------------------------------------------------------------------------------

Author:		Jason [Zygote] Brownlee
Email:		hop_cha@hotmail.com
Web:		http://planetquake.com/humandebris
Date:		31/October/2000
File:		QuakeServerCommunication.java
Version:	2

Description:
This object is designed to be reused many times. The best implementation would be to have
the "QueryServer" method called multiple times in a loop. It takes strings so that input
can come from textfields or an ascii file.
I have included DEBUG support which will dsiplay all errors, exceptions and warnings to
stdout (be it a java console or dos window).
*/

/*
CONSTRUCTOR:
--------------------------------------------------------------------------------
public QuakeServerCommunication(int timeout, boolean debug)


METHOD LIST:
--------------------------------------------------------------------------------
public void SetTimeout(int t)
public void SetPort(String p)
public void SetHost(String h)
public void SetDebug(boolean d)
public void SetHostAndPort(String h, String p)
public String GetCommandList()
public String GetIPAddress()
public String GetHostName()
public long GetPing()
public String QueryServer(String s_host, String s_port, String s_cmd)
public String QueryServer(String s_cmd)
private String ServerCommunication(String command) throws Exception
*/

import java.net.DatagramSocket;
import java.net.DatagramPacket;
import java.net.InetAddress;
//Exceptions
import java.io.IOException;
import java.io.InterruptedIOException;
import java.net.SocketException;
import java.net.UnknownHostException;

class QuakeServerCommunication
{
	//Public Constants
	public final static String GETINFO = "getinfo";
	public final static String GETSTATUS = "getstatus";
	public final static String GETCHALLENGE = "getchallenge";
	public final static String CONNECT = "connect";
	public final static String RCON = "rcon";
	public final static int DEFAULT_PORT = 27960;

	//Private variables
	private long ping_time = 0;			//ping time
	private boolean debug = false;		//print debug info
	private int timeout = 0;			//socket timeout
	private InetAddress host=null;		//Q3 server
	private int port = DEFAULT_PORT;	//port number

	/*
	Constructor
	----------------------------------------------------------------------------------------
	*/
	public QuakeServerCommunication(int t, boolean d)
	{
		//must be between 1 and 10 seconds
		if(t<1000 || t>10000)
		{
			timeout=3000;
			if(debug)
			{
				System.out.println("WARNING: Timeout must be (1-10) seconds. Set to 3 seconds(300ms).");
			}
		}else{
			timeout=t;
		}
		debug = d;
	}

	/*
	Set the timeout (between 1-10 seconds)
	----------------------------------------------------------------------------------------
	*/
	public void SetTimeout(int t)
	{
		if(t<1000 || t>10000)
		{
			if(debug)
			{
				System.out.println("WARNING: Timeout must be (1-10) seconds.");
			}
		}else{
			timeout=t;
		}
	}

	/*
	Set the port
	----------------------------------------------------------------------------------------
	*/
	public void SetPort(String p)
	{
		try{
			port=Integer.parseInt(p);
		}catch(NumberFormatException NFE)
		{
			if(debug)
			{
				System.out.println("EXCEPTION: "+NFE.toString());
				System.out.println("WARNING: Port out of range, set to"+DEFAULT_PORT+".");
			}
			port = DEFAULT_PORT;
		}
	}

	/*
	Set the host
	----------------------------------------------------------------------------------------
	*/
	public void SetHost(String h)
	{
		try{
			host = InetAddress.getByName(h);
		}catch(Exception E)
		{
			if(debug){
				System.out.println("EXCEPTION: "+E.toString());
			}
			host = null;
		}
	}

	/*
	Set the HOST and PORT
	----------------------------------------------------------------------------------------
	*/
	public void SetHostAndPort(String h, String p)
	{
		//HOST
		try{
			host = InetAddress.getByName(h);
		}catch(Exception E)
		{
			if(debug){
				System.out.println("EXCEPTION: "+E.toString());
			}
			host = null;
		}
		//PORT
		try{
			port=Integer.parseInt(p);
		}catch(NumberFormatException NFE)
		{
			if(debug)
			{
				System.out.println("EXCEPTION: "+NFE.toString());
				System.out.println("WARNING: Port out of range, set to"+DEFAULT_PORT+".");
			}
			port = DEFAULT_PORT;
		}
	}

	/*
	Get the command list
	----------------------------------------------------------------------------------------
	*/
	public String GetCommandList()
	{
		return new String(GETINFO+" "+GETSTATUS+" "+GETCHALLENGE+" "+CONNECT+" "+RCON);
	}


	/*
	Get the HOST ADDRESS
	----------------------------------------------------------------------------------------
	*/
	public String GetHostName()
	{
		if(host==null)
		{
			return null;
		}
		return host.getHostName();
	}


	/*
	Get the ip address
	----------------------------------------------------------------------------------------
	*/
	public String GetIPAddress()
	{
		if(host==null)
		{
			return null;
		}
		return host.getHostAddress();
	}

	/*
	Get the ping value
	----------------------------------------------------------------------------------------
	*/
	public long GetPing()
	{
		return ping_time;
	}


	/*
	Switch the debug feature on or off
	----------------------------------------------------------------------------------------
	*/
	public void SetDebug(boolean d)
	{
		debug = d;
	}

	/*
	Returns a string (ready to be tokenized) from the server, or null on error.
	----------------------------------------------------------------------------------------
	*/
	public String QueryServer(String s_host, String s_port, String s_cmd)
	{
		//test parameters
		if(s_host.length()<=0 || s_port.length()<=0 || s_cmd.length()<=0)
		{
			if(debug)
			{
				System.out.println("ERROR: Bad Parameters!");
			}
			return null;
		}
		//build numbers
		try{
			port=Integer.parseInt(s_port);
		}catch(NumberFormatException NFE)
		{
			if(debug){
				System.out.println("EXCEPTION: "+NFE.toString());
				System.out.println("WARNING: Port out of range, set to"+DEFAULT_PORT+".");
			}
			port = DEFAULT_PORT;
		}
		//get the inetaddress
		try{
			host = InetAddress.getByName(s_host);
		}catch(Exception E)
		{
			if(debug){
				System.out.println("EXCEPTION: "+E.toString());
			}
			return null;
		}
		//test port
		if(port<=1024 || port>=60000)
		{
			port = DEFAULT_PORT;;
			if(debug)
			{
				System.out.println("WARNING: Port out of range, set to"+DEFAULT_PORT+".");
			}
		}
		try
		{
			return ServerCommunication(s_cmd);
		}catch(Exception e)
		{
			if(debug)
			{
				System.out.println("EXCEPTION: "+e.toString());
			}
			return null;
		}
	}


	/*
	Query the server and assume PORT and HOST are already set.
	----------------------------------------------------------------------------------------
	*/
	public String QueryServer(String s_cmd)
	{
		String temp;				//return string

		//test parameters
		if(s_cmd.length()<=0)
		{
			if(debug)
			{
				System.out.println("ERROR: Bad Parameters!");
			}
			return null;
		}
		if(host==null)
		{
			if(debug)
			{
				System.out.println("ERROR: There is no host set!");
			}
			return null;
		}
		//test port
		if(port<=1024 || port>=60000)
		{
			port = DEFAULT_PORT;
			if(debug)
			{
				System.out.println("WARNING: Port out of range, set to"+DEFAULT_PORT+".");
			}
		}

		try
		{
			return ServerCommunication(s_cmd);
		}catch(Exception e)
		{
			if(debug)
			{
				System.out.println("EXCEPTION: "+e.toString());
			}
			return null;
		}
	}

	/*
	Send and receives a packet from a Quake server. Returns the string from the server.
	Returns null on error.
	----------------------------------------------------------------------------------------
	*/
	private String ServerCommunication(String command) throws Exception
	{
		DatagramSocket socket = null;			// Datagram Socket
		DatagramPacket packet = null;			// Datagram Packet
		byte[] packet_buffer = new byte[65507];	// Packet Buffer
		long socket_start_time = 0;				// Start Time

		//build command
 		String out = "xxxx"+command;
		byte [] buff = out.getBytes();
      	buff[0] = (byte)0xff;
      	buff[1] = (byte)0xff;
      	buff[2] = (byte)0xff;
      	buff[3] = (byte)0xff;

		//create the socket
		try{
			socket = new DatagramSocket();
		}catch(SocketException SE)
		{
			if(debug){
				System.out.println("EXCEPTION: "+SE.toString());
			}
			return null;
		}catch(SecurityException SE)
		{
			if(debug){
				System.out.println("EXCEPTION: "+SE.toString());
			}
			return null;
		}

		//set timeout
		try{
			socket.setSoTimeout(timeout);
		}catch(SocketException SE)
		{
			if(debug){
				System.out.println("EXCEPTION: "+SE.toString());
			}
			return null;
		}

		//build packet
		packet = new DatagramPacket(buff, buff.length, host, port);

		//get the time
		socket_start_time = System.currentTimeMillis();

		//send the packet
		try{
			socket.send(packet);
		}catch(IOException IOE)
		{
			if(debug){
				System.out.println("EXCEPTION: "+IOE.toString());
			}
			return null;
		}catch(SecurityException SE)
		{
			if(debug){
				System.out.println("EXCEPTION: "+SE.toString());
			}
			return null;
		}

		//build receive packet
		packet = new DatagramPacket(packet_buffer, packet_buffer.length);

		//receive packet
		try{
			socket.receive(packet);
		}catch(IOException IOE)
		{
			if(debug){
				System.out.println("EXCEPTION: "+IOE.toString());
			}
			return null;
		}

		//calculate ping
		ping_time = (System.currentTimeMillis() - socket_start_time);
		if(debug)
		{
			System.out.println("PING: "+ping_time);
		}

		//cast the bitch to chars
		char c[] = new char[packet.getLength()];
		for(int i=0; i<packet.getLength(); i++)
		{
			c[i] = (char)packet_buffer[i];
		}

		//close the socket
		socket.close();

		return String.valueOf(c);
	}
}