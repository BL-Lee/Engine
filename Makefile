
#INCLUDES=-Idependencies/GLFW/include -Idependencies/GLEW/include -Idependencies/OBJ-Loader -Idependencies/stb -Idependencies/imgui-master -Idependencies/msdfgen-atlas-gen -Idependencies/perlin-noise/src
#IMGUI_DIR=dependencies/imgui-master
#LIBS=dependencies/GLFW/lib-x86_64/libglfw3.a dependencies/GLEW/lib/libGLEW.a $(IMGUI_DIR)/imgui.a 

#IMGUI_OBJ=$(IMGUI_DIR)/build/imgui.o $(IMGUI_DIR)/build/imgui_demo.o $(IMGUI_DIR)/build/imgui_draw.o $(IMGUI_DIR)/build/imgui_tables.o $(IMGUI_DIR)/build/imgui_widgets.o $(IMGUI_DIR)/build/imgui_impl_glfw.o $(IMGUI_DIR)/build/imgui_impl_opengl3.o

#Application : src/*
#	g++ -w -O2 -DDEBUG_ASSERT src/Application.cpp -o Application $(INCLUDES) $(LIBS) -framework Cocoa -framework OpenGL -framework IOKit	
#debug: src/*
#	g++ -g -DDEBUG_ASSERT src/Application.cpp -o Application $(INCLUDES) $(LIBS) -framework Cocoa -framework OpenGL -framework IOKit

#$(IMGUI_DIR)/build/%.o:$(IMGUI_DIR)/%.cpp
#	g++ -c -o $@ $^ $(INCLUDES)
#IMGUI_LIB:$(IMGUI_OBJ)
#	ar rcs $(IMGUI_DIR)/imgui.a $^

All:
	make -C build
