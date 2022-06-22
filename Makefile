CFLAGS=
LDFLAGS=-lpthread  -lopencv_imgproc -lopencv_highgui -lopencv_core -lopencv_ml -lopencv_videoio -lopencv_imgcodecs -lraspicam -lraspicam_cv -lavformat -lswscale -lavutil -lavcodec
LIBDIR=-L/usr/local/lib 
INCDIR= 


default : Encoder
	
Encoder : Encoder.cpp
	g++  -o $@ $^  $(INCDIR) $(LIBDIR) $(LDFLAGS) 


clean :
	rm -f *.o Encoder
