ToolDAQPath=/ToolDAQApplication/ToolDAQ

ZMQLib= -L $(ToolDAQPath)/zeromq-4.0.7/lib -lzmq
ZMQInclude= -I $(ToolDAQPath)/zeromq-4.0.7/include/

BoostLib= -L $(ToolDAQPath)/boost_1_66_0/install/lib -lboost_date_time -lboost_serialization -lboost_iostreams
BoostInclude= -I $(ToolDAQPath)/boost_1_66_0/install/include


all:
	g++ src/*.cpp -I ./include $(ZMQInclude) $(ZMQLib) $(BoostInclude) $(BoostLib) 
