CPPFLAGS = -nologo -EHsc -Itclap/include

woman.exe: main.obj pipe.obj
	cl $(CPPFLAGS) $^ -Fe:$@

main.obj: main.cpp
	cl $(CPPFLAGS) -c $^ -Fe:$@

pipe.obj: pipe.cpp
	cl $(CPPFLAGS) -c $^ -Fe:$@

@PHONY: clean

clean:
	rm -rf main.obj pipe.obj woman.exe
