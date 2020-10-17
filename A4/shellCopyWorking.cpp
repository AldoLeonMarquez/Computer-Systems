#include <iostream>
#include <algorithm>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <fstream>
#include <ncurses.h>



using namespace std;


char cwd[256];
string previous;
vector<int> Child;

string trim (string input){
    int i=0;
    while (i < input.size() && input [i] == ' ')
        i++;
    if (i < input.size())
        input = input.substr (i);
    else{
        return "";
    }
    
    i = input.size() - 1;
    while (i>=0 && input[i] == ' ')
        i--;
    if (i >= 0)
        input = input.substr (0, i+1);
    else
        return "";
    
    return input;
    

}

vector<string> split (string line, string separator=" "){
    vector<string> result;
    while (line.size()){
        size_t found = line.find(separator);
		
		size_t found2 = line.find("\"");
		size_t found3 = line.find("\"",found2+1);
		
		size_t found4 = line.find("\'");
        size_t found5 = line.find("\'",found4+1);
		
		if(found2 != string::npos && found3 != string::npos && found > found2){
			
			found = line.find(separator,found3);
		}
		else if(found4 != string::npos && found5 != string::npos && found > found4){
			
			found = line.find(separator,found5);
		}
		
        if (found == string::npos){
            string lastpart = trim (line);
            if (lastpart.size()>0){
                result.push_back(lastpart);
            }
            break;
        }
        string segment = trim (line.substr(0, found));
        //cout << "line: " << line << "found: " << found << endl;
        line = line.substr (found+1);

        //cout << "[" << segment << "]"<< endl;
        if (segment.size() != 0) 
            result.push_back (segment);

        
        //cout << line << endl;
    }
    return result;
}

char** vec_to_char_array (vector<string> parts){
    char ** result = new char * [parts.size() + 1]; // add 1 for the NULL at the end
    for (int i=0; i<parts.size(); i++){
        // allocate a big enough string
        result [i] = new char [parts [i].size() + 1]; // add 1 for the NULL byte
        strcpy (result [i], parts[i].c_str());
    }
    result [parts.size()] = NULL;
    return result;
}

void execute (string command){
	
    vector<string> argstrings = split (command, " "); // split the command into space-separated parts
	char** args = vec_to_char_array (argstrings);// convert vec<string> into an array of char*
	
	bool IN= false;
	bool OUT= false;
	bool BG = false;
	
	for(int i = 0; i<argstrings.size();i++){
		//cout<<argstrings[i]<<" I = "<<i<<endl;
		size_t foundTemp = argstrings[i].find("<");
		size_t foundTemp2 = argstrings[i].find(">");
		size_t foundTemp3 = argstrings[i].find("&");
		
		if(foundTemp != string::npos){IN = true;}
		if(foundTemp2 != string::npos){OUT = true;}
		if(foundTemp3 != string::npos){BG = true;}
		
	}
	
	
    if(argstrings[0] ==  "cd"){ //working
		string cdType = argstrings[1];
		
		if (cdType == "-"){//previous
			string TempPrevious = getcwd(cwd, sizeof(cwd)); 
			cout<<previous<<endl;
			chdir(previous.c_str());
			previous = TempPrevious;
		}
		else if (cdType == ".." || cdType == "../" ){ //level up 
			//cout<<"EZ Clap"<<endl;
			previous = getcwd(cwd, sizeof(cwd)); //Update previous
			string TempPrevious = getcwd(cwd, sizeof(cwd)); 
			
			vector<string> levels = split (cdType, "/");
			
			vector<string> DirParts = split (TempPrevious, "/");
			
			for(int i = 0; i < levels.size() ; i++){
				DirParts.pop_back();
			}
			string Dir = "/";
			for(int i = 0; i < DirParts.size() ; i++){
				Dir += DirParts[i];
				Dir += "/";
			}
			if(levels[levels.size()-1] != ".."){
				
				Dir += levels[levels.size()-1];
			}
			//cout<<Dir<<" wut"<<endl;
			chdir(Dir.c_str());
		}
		else if(cdType[0] == '/'){
			previous = getcwd(cwd, sizeof(cwd)); //Update previous
			chdir(cdType.c_str());
		}
		else{
			
			previous = getcwd(cwd, sizeof(cwd)); 
			string Dir = cwd ;
			Dir += "/";
			Dir += args[1];
			chdir(Dir.c_str());
			
			
		}
	}
	
	else if (BG){
		string Temp ="";
	
		for (int i = 0 ; i < argstrings.size(); i++){
			Temp+=argstrings[i]+" ";
		}
		vector<string> Background = split(Temp,"&");
		

		for(int i = 0; i < Background.size();i++){
			//cout<<Background[i]<<endl;
			
			int sPid;
			int pid=fork();
			if(pid!=0){Child.push_back(pid);}
			if(pid == 0) {
				
				//cout<<getpid()<<" inside BG"<<endl;
				vector<string> Temp = split(Background[i]," ");
				
				for(int i = 0;i<Temp.size();i++){
					
					
				}
				char** argsE = vec_to_char_array (Temp);
				execvp (argsE[0], argsE);
				
				
			}
			else{
				int a = waitpid(pid,0,WNOHANG);
				if(a == pid){
					
					for(int i = 0; i< Child.size();i++){
						if(Child[i] == a){
							Child.erase(Child.begin()+i); //if it finished inmidiately 
						}
					}
				}
			}
			
		}
	}
	
	else if(IN || OUT ){
		
		
		if(IN && OUT){
			string Right = getcwd(cwd, sizeof(cwd)); // Final output destination
			Right += "/";
			vector<string> Copy = argstrings;
			vector<string> Left;
			
			for(int i = 0; i < Copy.size(); i++){
				
				if(Copy[i][0] == '\"' || Copy[i][0] == '\'' ){
					Copy[i].erase(Copy[i].begin());
					Copy[i].erase(Copy[i].end()-1);
					
				}
			}
			
			for(int i = 0; i < Copy.size();i++){
				if(Copy[i] == ">"){
					Right += Copy[i+1];    //Get string Right to the ">" i.e output file
					i=Copy.size();
				}
			}
			
			string RightIn = getcwd(cwd, sizeof(cwd)); // Input File 
			RightIn += "/";
			vector<string> Copy2 = argstrings;
			for(int i = 0; i < Copy2.size();i++){
				
				if(Copy2[i] == "<"){
					RightIn += Copy2[i+1];    //Get string Right to the "<" i.e input file
					i=Copy2.size();
				}
				else {
					Left.push_back(Copy2[i]);
				}
			}
				for(int i = 0; i < Left.size();i++){
				
			}
			// Open File for Write
			int fd = open(RightIn.c_str(),O_RDONLY | S_IRUSR | S_IREAD | S_IRGRP);
			dup2(fd,0);
			int fd1 = open(Right.c_str(),O_CREAT | O_WRONLY | O_TRUNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
			dup2(fd1,1);
			
			// Open File for read
			
			
			
			char** argsE = vec_to_char_array (Left);
			execvp (argsE[0], argsE);

			//close(fd);
			close(fd1);
			
			
		}
		else if(IN && !OUT){
			vector<string> Copy = argstrings;
			string Left=getcwd(cwd, sizeof(cwd));
			Left += "/";
			
			for(int i = 0; i < Copy.size(); i++){
				
				if(Copy[i][0] == '\"' || Copy[i][0] == '\'' ){
					Copy[i].erase(Copy[i].begin());
					Copy[i].erase(Copy[i].end()-1);
					
				}
			}
			for(int i = 0; i < Copy.size(); i++){
				if(Copy[i] == "<"){
					Left += Copy[i+1];
					Copy.pop_back();
					Copy.pop_back();
					
				}
			}
		
			int fd = open(Left.c_str(),O_RDONLY | S_IRUSR | S_IREAD | S_IRGRP);
			dup2(fd,0);
			
			char** argsE = vec_to_char_array (Copy);
			
			execvp (argsE[0], argsE);
			close(fd);
			
		}
		else if(OUT && !IN){
			vector<string> Copy = argstrings;
			vector<string> Left;
			string Right = getcwd(cwd, sizeof(cwd));
			
			Right += "/";
			for(int i = 0; i < Copy.size(); i++){
				
				if(Copy[i][0] == '\"' || Copy[i][0] == '\'' ){
					Copy[i].erase(Copy[i].begin());
					Copy[i].erase(Copy[i].end()-1);
					
				}
				//cout<<Copy[i]<<" lesgo"<<endl;
			}
			
			for(int i = 0; i < Copy.size();i++){
				if(Copy[i] != ">"){
					Left.push_back(Copy[i]);
				}
				else if(Copy[i] == ">"){
					Right += Copy[i+1];
					i=Copy.size();
				}
			}
			int fd = open(Right.c_str(),O_CREAT | O_WRONLY | O_TRUNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
			dup2(fd,1);
			char** argsE = vec_to_char_array (Left);
			
			execvp (argsE[0], argsE);
			close(fd);
		}
	}
	else if(argstrings[0] ==  "echo"){ //working 
		
		string TrimmedEcho = argstrings[1];
		
		string Text = ""; 
		for(int i = 1; i<argstrings.size(); i++){
			Text += argstrings[i];
		}
		
		if( (Text[0] == '\'' || Text[0] == '\"') && (Text[Text.size()-1] == '\'' || Text[Text.size()-1] == '\"')){
			Text.erase(Text.begin());
			Text.erase(Text.end()-1);
		}
		argstrings[1] = Text;
		char** argsE = vec_to_char_array (argstrings);
		execvp (argsE[0], argsE);
	}
	else if(argstrings[0] ==  "awk"){
		string temp = argstrings[1];
		
		temp.erase(temp.begin());
		temp.erase(temp.end()-1);
		
		argstrings[1] = temp;
		char** argsE = vec_to_char_array (argstrings);
		execvp (argsE[0], argsE);
	}
	else {
		execvp (args[0], args);
	}
	
}



int main (){
	string commandline = "";
	getcwd(cwd, sizeof(cwd)); //Store 
	previous = cwd;
    while (true){ // repeat this loop until the user presses Ctrl + C
        /*get from STDIN, e.g., "ls  -la |   grep Jul  | grep . | grep .cpp" */
        // split the command by the "|", which tells you the pipe levels
		getcwd(cwd, sizeof(cwd)); //Store 
		cout<<"Root@Pa-4-Shell:"<<cwd<<"# ";
		
		for(int i = 0; i< Child.size();i++){
			int a = waitpid(Child[i],0,WNOHANG);
			
			if (a == Child[i]){
				Child.erase(Child.begin()+i);
			}
		}
		getline(cin,commandline);
       
	
        if (commandline =="jobs"){ //Background processes 
			cout<<Child.size()<<" Processes Running in the Background"<<endl;
			for(int i = 0; i< Child.size();i++){
				cout<<i+1<<"    PID: "<<Child[i]<<endl;
			}
			continue;
		}
		bool DOLLA = false;
		string D="";
		string result;
		int Index$=0;
		string LeftLine="";
		string RightLine="";
		for(int i = 0 ;i<commandline.length();i++){
				if(commandline[i] == '$' &&  commandline[i+1] == '('){
					Index$ = i;
					DOLLA = true;
					for(int j = i+2 ;i<commandline.length();j++){
						if (commandline[j]  == ')'){
							for(int k = j +1;k< commandline.length();k++){
								RightLine+=commandline[k];
							}
							j=9999;
							i=9999;
						}
						else{
						D += commandline[j];
						}
					}						
				}
				else {
					LeftLine+=commandline[i];
				}
		}
	
		if( DOLLA ){
			
			vector<string> dollar = split (D, "|");
			int Origin = dup(0);
			int fdo = open("temp.txt",O_CREAT | O_WRONLY | O_TRUNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
			
			for (int i=0; i<dollar.size(); i++){
				// make pipe
	
				int fd[2];
				pipe(fd);
				int pid = fork();
				if (pid == 0){
					// redirect output to the next level
					
					// unless this is the last level
					if (i < dollar.size() - 1){
						// redirect STDOUT to fd[1], so that it can write to the other side
						dup2(fd[1],1);
						close (fd[1]);   // STDOUT already points fd[1], which can be closed
					}
					if( i == dollar.size() - 1){
						dup2(fdo,1);
					}
					
					//execute function that can split the command by spaces to 
					// find out all the arguments, see the definition
					
					execute (dollar [i]); // this is where you execute
					
				}else{
							  // wait for the child process
					waitpid(pid,0,0);
					// then do other redirects
					dup2(fd[0],0);
					close(fd[1]);
				}
			}
			
			ifstream infile;
			infile.open ("temp.txt");
			getline(infile,result);
			infile.close();
			 if( remove( "temp.txt" ) != 0 ){
				//perror( "Error deleting file" );
			 }
			  else{
				//puts( "File successfully deleted" );
			  }
				
				DOLLA = false;
				close(fdo);
				dup2(Origin,0);
			
				commandline = LeftLine +result +RightLine;
		}
		vector<string> tparts = split (commandline, "|");
		
		
		int Origin = dup(0);
        for (int i=0; i<tparts.size(); i++){
            // make pipe
			int fd[2];
			pipe(fd);
			int pid = fork();
			if (pid == 0){
                // redirect output to the next level
				
                // unless this is the last level
                if (i < tparts.size() - 1){
                    // redirect STDOUT to fd[1], so that it can write to the other side
					dup2(fd[1],1);
                    close (fd[1]);   // STDOUT already points fd[1], which can be closed
                }
				
                //execute function that can split the command by spaces to 
                // find out all the arguments, see the definition
				
                execute (tparts [i]); // this is where you execute
				
            }else{
				          // wait for the child process
				waitpid(pid,0,0);
				// then do other redirects
				dup2(fd[0],0);
				close(fd[1]);
            }
	
        }
	//restore the standard input for the next command line
	//Or else it would get the line from the last proces of the previous line 
	dup2(Origin,0);
    }
	
}