/*
Author:		Jason [Zygote] Brownlee
Email:		hop_cha@hotmail.com
Web:		http://planetquake.com/humandebris
Date:		10/October/2000
File:		QuakeServerCommunication.java
Version:	2

Description:
A collection of methods for processing strings returned from Q3 servers.
*/

import java.util.StringTokenizer;
import java.util.NoSuchElementException;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileNotFoundException;

public class Q3Utils
{
	private boolean debug = false;

	/*
	Constructor
	----------------------------------------------------------------------------------------
	*/
	public Q3Utils(boolean d)
	{
		debug = d;
	}

	/*
	Process a raw "getinfo" string into a PING array, Gamespy style. Returns null on error.
	If any element does not get a value then its null. Check for this on processing.
	----------------------------------------------------------------------------------------
	*/
	public String[] BuildPingArray(String s)
	{
		if(s==null)
		{
			System.out.println("ERROR: Sent a null string");
			return null;
		}
		/*
		[0] - Protocol
		[1] - Hostname (Game Name)
		[2] - Map Name
		[3] - Clients / MaxClients
		[4] - Gametype (STRING)
		[5] - Pure
		*/
		String ping_array[] = new String[6];
		StringTokenizer tokenizer = new StringTokenizer(s,"\\");
		String temp = null;
		String clients = "???";
		String maxclients = "???";
		int gametype = 0;

		try
		{
			while(tokenizer.hasMoreTokens()){
				temp=tokenizer.nextToken();
				if(temp.compareTo("protocol") == 0) {
					ping_array[0] = tokenizer.nextToken();
				}else if(temp.compareTo("hostname") == 0) {
					ping_array[1] = tokenizer.nextToken();
				}else if(temp.compareTo("mapname") == 0) {
					ping_array[2] = tokenizer.nextToken();
				}else if(temp.compareTo("clients") == 0) {
					clients = tokenizer.nextToken();
				}else if(temp.compareTo("sv_maxclients") == 0) {
					maxclients = tokenizer.nextToken();
				}else if(temp.compareTo("gametype") == 0) {
					try{
						gametype = Integer.parseInt(tokenizer.nextToken());
					}catch(NumberFormatException NFE){
						if(debug)
						{
							System.out.println("EXCEPTION: "+NFE.toString());
							ping_array[4] = tokenizer.nextToken();
						}
					}
					switch(gametype){
					case 0: 	ping_array[4] = "FFA";
								break;
					case 1: 	ping_array[4] = "Tournament";
								break;
					case 2: 	ping_array[4] = "Single Player";
								break;
					case 3: 	ping_array[4] = "Team DM";
								break;
					case 4: 	ping_array[4] = "CTF";
								break;
					default: 	ping_array[4] = "???";
								break;
					}
				}else if(temp.compareTo("pure") == 0) {
					ping_array[5] = tokenizer.nextToken();
				}
			}
		}catch(NoSuchElementException NSEE)
		{
			if(debug)
			{
				System.out.println("EXCEPTION: "+NSEE.toString());
			}
			return null;
		}
		ping_array[3] = clients+"/"+maxclients;
		return ping_array;
	}

	/*
	Process a raw "getinfo" string into a 2d array [variable][value].
	Returns null on error.
	----------------------------------------------------------------------------------------
	*/
	public String[][] BuildInfoArray(String s)
	{
		if(s==null)
		{
			System.out.println("ERROR: Sent a null string");
			return null;
		}
		StringTokenizer tokenizer = new StringTokenizer(s,"\\");
		String info_array[][] = new String[((tokenizer.countTokens()-1)/2)][2];
		int i = 0;
		try
		{
			tokenizer.nextToken(); //remove header
			for(i=0; tokenizer.hasMoreTokens(); i++)
			{
				info_array[i][0] = tokenizer.nextToken();
				info_array[i][1] = tokenizer.nextToken();
			}
		}catch(NoSuchElementException NSEE)
		{
			if(debug)
			{
				System.out.println("EXCEPTION: [BuildInfoArray] "+NSEE.toString());
			}
			return null;
		}
		return info_array;
	}

	/*
	Process a "status" string into a 2d array [variable][value].
	Returns null on error.
	NOTE: The "Players" section MUST not be included in the raw string!!!
	----------------------------------------------------------------------------------------
	*/
	public String[][] BuildStatusArray(String s)
	{
		if(s==null)
		{
			System.out.println("ERROR: Sent a null string");
			return null;
		}
		StringTokenizer tokenizer = new StringTokenizer(s,"\\");
		String status_array[][] = new String[((tokenizer.countTokens())/2)][2];
		int i = 0;
		try
		{
			for(i=0; tokenizer.hasMoreTokens(); i++)
			{
				status_array[i][0] = tokenizer.nextToken();
				status_array[i][1] = tokenizer.nextToken();
			}
		}catch(NoSuchElementException NSEE)
		{
			if(debug)
			{
				System.out.println("EXCEPTION: [BuildStatusArray] "+NSEE.toString());
			}
			return null;
		}
		return status_array;
	}


	/*
	Process a "players" string into a 2d array [score][ping][name].
	Returns null on error.
	NOTE: The "Status" section MUST not be included in the raw string!!!
	----------------------------------------------------------------------------------------
	*/
	public String[][] BuildPlayersArray(String s)
	{
		if(s==null)
		{
			System.out.println("ERROR: Sent a null string");
			return null;
		}
		StringTokenizer token = new StringTokenizer(s,"\n");
		StringTokenizer space_tokens = null;
		String details_array[][] = new String[token.countTokens()][3];
		String temp;
		int i = 0;

		try{
			for(i=0; token.hasMoreTokens(); i++)
			{
				temp = token.nextToken();
				space_tokens = new StringTokenizer(temp," ");
				details_array[i][0] = space_tokens.nextToken();
				details_array[i][1] = space_tokens.nextToken();
				details_array[i][2] = temp.substring(temp.indexOf("\"")+1, temp.length()-1);
			}
		}catch(Exception e){
			if(debug)
			{
				System.out.println("EXCEPTION: [BuildPlayersArray] "+e.toString());
			}
			return null;
		}
		return details_array;
	}

	/*
	Process a VERY raw "getstatus" string into a 2 element array. Returns null on error.
	[0] = status string
	[1] = players string
	----------------------------------------------------------------------------------------
	*/
	public String[] SeperateStatusArray(String s)
	{
		if(s==null)
		{
			System.out.println("ERROR: Sent a null string");
			return null;
		}
		String array[] = new String[2];
		int pos1, pos2;

		//get the pos of the newline afer the id
		if((pos1=s.indexOf("\n"))==-1)
		{
			if(debug)
			{
				System.out.println("ERROR: [SeperateStatusArray] Could not get 1st newline!");
			}
			return null;
		}
		//get the pos of the newline where the players string finishes
		if((pos2=s.indexOf("\n",pos1+1))==-1)
		{
			if(debug)
			{
				System.out.println("ERROR: [SeperateStatusArray] Could not get 2nd newline!");
			}
			return null;
		}

		//set the status string
		try
		{
			array[0] = s.substring(pos1+1, pos2);
		}catch(IndexOutOfBoundsException IOOBE)
		{
			if(debug)
			{
				System.out.println("EXCEPTION: [SeperateStatusArray] "+IOOBE.toString());
			}
			return null;
		}

		pos1 = pos2;
		if((pos2=s.lastIndexOf("\n"))==-1)
		{
			if(debug)
			{
				System.out.println("ERROR: [SeperateStatusArray] Could not get last newline!");
			}
			return null;
		}

		//set players string
		try
		{
			array[1] = s.substring(pos1+1, pos2);
		}catch(IndexOutOfBoundsException IOOBE)
		{
			if(debug)
			{
				System.out.println("EXCEPTION: [SeperateStatusArray] "+IOOBE.toString());
			}
			array[1] = null;
			return array;
		}
		return array;
	}


	/*
	Returns a Buffered reader with access to a file, or null on error.
	----------------------------------------------------------------------------------------
	*/
	private BufferedReader GetFile(String s)
	{
		FileReader fr;
		try
		{
			fr = new FileReader(s);
		}catch(FileNotFoundException FNFE)
		{
			if(debug)
			{
				System.out.println("EXCEPTION: "+FNFE.toString());
			}
			return null;
		}
		return new BufferedReader(fr);
	}


	/*
	Build a 2d array of favourites [servers][ports] from a q3config.cfg file.
	The total number of favourites is a parameter, but the maximum that q3 allows is 16.
	----------------------------------------------------------------------------------------
	*/
	public String[][] BuildFavouritesArray(String filename, int total_favourites)
	{
		BufferedReader file;
		String favourites[][] = new String[total_favourites][2];
		String s = null;
		String temp1 = null;
		String temp2 = null;
		int pos1 = 0;
		int pos2 = 0;
		int count = 1;

		if( (file=GetFile(filename)) == null )
		{
			if(debug)
			{
				System.out.println("ERROR: File not found!");
			}
			return null;
		}

		try
		{
			//keep reading while EOF && we have not got enough favourites
			while((s=file.readLine())!=null && count!=total_favourites){
				if(s.startsWith("seta server")){
					if( (pos1=s.indexOf('\"')) == -1){
						continue; //bail
					}
					if( (pos2=s.indexOf(':')) == -1){
						continue; //bail
					}
					temp1 = s.substring(pos1+1, pos2);
					if( (pos1=s.lastIndexOf('\"')) == -1){
						continue; //bail
					}
					temp2 = s.substring(pos2+1, pos1);
					favourites[count][0] = temp1;
					favourites[count][1] = temp2;
					count++;
				}
			}
			//close the stream
			file.close();
		}catch(Exception e){
			if(debug)
			{
				System.out.println("EXCEPTION: "+e.toString());
			}
			return null;
		}
		return favourites;
	}
}