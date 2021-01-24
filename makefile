# makefile for CyCells

CC = g++
CFLAGS = -O1 -Wall -Winline 
COMMONOBJ = tissue.o cells.o cellType.o sense.o molecule.o \
	random.o history.o fileDef.o fileInit.o tallyActions.o action.o
WXOBJ = app.o simFrame.o simView.o historyView.o simView3D.o dataDialog.o 
LDLIBS = -lwx_gtk_gl -lwx_gtk -lGL


CyCells : main.o $(COMMONOBJ)
	$(CC) $(CFLAGS) -o CyCells main.o $(COMMONOBJ) 

wxCyCells : $(WXOBJ) $(COMMONOBJ)
	$(CC) $(CFLAGS) -o wxCyCells $(WXOBJ) $(COMMONOBJ) $(LDLIBS)

$(COMMONOBJ) : %.o: %.cc
	$(CC) $(CFLAGS) -c -o $@ $< 

$(WXOBJ) : %.o: %.cc
	$(CC) $(CFLAGS) -c `wx-config --cxxflags` -o $@ $< 

main.o : tissue.h history.h fileDef.h fileInit.h
app.o : app.h simFrame.h
tissue.o : tissue.h cells.h molecule.h random.h
cells.o : cells.h cellType.h cell.h simPoint.h random.h
cellType.o : cellType.h cell.h random.h sense.h action.h condition.h
molecule.o : molecule.h array3D.h simPoint.h 
fileDef.o : fileDef.h tissue.h molecule.h cellType.h sense.h rate.h action.h \
	process.h condition.h 
fileInit.o : fileInit.h tissue.h
history.o : history.h tissue.h
simFrame.o : simFrame.h simView.h historyView.h simView3D.h \
	tissue.h history.h fileDef.h fileInit.h
simView.o : simView.h
simView3D.o : simView3D.h simView.h tissue.h
historyView.o : historyView.h simView.h history.h
random.o : random.h
dataDialog.o : dataDialog.h

clean : 
	rm -f *.o 


