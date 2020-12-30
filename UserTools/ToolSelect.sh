#!/bin/bash

if ! command -v dialog &> /dev/null
then
    echo "dialog package needed for operation. Please install then re-run script"
    exit
fi


#cat Factory.cpp | grep -v '/' |grep if| awk '{print $4}' | sed s:';':: > on
num=1

#ls */*.cpp | sed s:/:' ': | awk '{print $2}' | sed s:.cpp:: | grep -v Factory
#ls -p | grep -v Factory | grep -v template | grep / | sed s:/::\

dialog --backtitle "Options" \
    --radiolist "Select (with spacebar) option:" 0 0 0 \
    1 "Activate/Deactivate Tools" on \
    2 "Restore Backup" off 2>tmpoption

if [ $? -eq 0 ]
then
    
    if [ `cat tmpoption` -eq 1 ]
    then
	
	dialog --checklist "Activate or Deactivate Tools with <spacebar>, Enter for OK and ESC for Cancel:" 0 0 0 \
	    `for Tool in \`ls */*.cpp | sed s:/:' ': | awk '{print $2}' | sed s:.cpp:: | grep -v Factory\`
do
    fin=0
    for current in \`cat Factory/Factory.cpp | grep -v '/' |grep if| awk '{print $4}' | sed s:';':: \`
    do
        if [ $Tool == $current ]
        then
            fin=1
            echo "$Tool $num on "
        fi
    done

    if [ $fin -eq 0 ]
    then
        echo "$Tool $num off "
    fi
    num=$(expr 1 + $num)
done` 2> tmptools
	
	if [ $? -eq 0 ]
	then
	    
	    dialog --title "Message"  --yesno "Backup existing configuration?" 0 0
	    if [ $? -eq 0 ]
	    then
		cp Unity.h Unity.bak.`date +%F.%T`
		cp Factory/Factory.cpp Factory/Factory.bak.`date +%F.%T`
	    fi
	    
	    rm -f Unity.h
	    for Tool in `cat tmptools`
	    do
		echo "#include <$Tool.h>" >> Unity.h
	    done
	    
	    echo "#include \"Factory.h\"

Tool* Factory(std::string tool) {
Tool* ret=0;

// if (tool==\"Type\") tool=new Type;" > Factory/Factory.cpp
	    for Tool in `cat tmptools`
	    do
		echo "if (tool==\"$Tool\") ret=new $Tool;" >> Factory/Factory.cpp
	    done
	    
	    echo "return ret;
}" >> Factory/Factory.cpp
	

	fi    
	
	rm -f tmptools
	   
	
    else

	dialog --radiolist "Activate or Deactivate Tools with <spacebar>, Enter for OK and ESC for Cancel:" 0 0 0 \
	    `for backup in \`ls *.bak.*\`
	 do
	echo $backup . off
	 done
	` 2> tmpbackup
	
	if [ $? -eq 0 ]
        then

	    if [ -s tmpbackup ]
	    then

		dialog --title "Message"  --yesno "Backup existing configuration?" 0 0
		if [ $? -eq 0 ]
		then
                    cp Unity.h Unity.bak.`date +%F.%T`
                    cp Factory/Factory.cpp Factory/Factory.bak.`date +%F.%T`
		fi
	
		cp `cat tmpbackup` Unity.h
		cp `cat tmpbackup | sed s:Unity:Factory/Factory:` Factory/Factory.cpp
	    fi
	    
	fi

	rm -f tmpbackup
    fi
    

fi

rm -f tmpoption
clear
