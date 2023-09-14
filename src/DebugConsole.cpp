#include "DebugConsole.h"

//#include "DebugConsoleStyle.cpp"
void initGeneralDebugInfo()
{
  globalDebugData.wrappedGraphIndex = 0;
  for (int i = 0 ; i < DEBUG_TIMING_HISTORY_LENGTH; i++)
    {
      globalDebugData.msHistory[i] = 0;
      globalDebugData.fpsHistory[i] = 0;
      globalDebugData.xAxis[i] = i;
    }
  globalDebugData.weightedMS = 0.0f;
  globalDebugData.weightedFPS = 0.0f;
  globalDebugData.showAABB = false;
  globalDebugData.showConsole = false;
  globalDebugData.arrowSelected = 0;
  globalDebugData.currentFolder = (char*)malloc(256);
}

void updateGeneralDebugInfo()
{
  f32 currMS = globalDeltaTime / globalTimeScale;
  f32 currFPS = 1.0f / currMS;
  globalDebugData.msHistory[globalDebugData.wrappedGraphIndex] = currMS;
  globalDebugData.fpsHistory[globalDebugData.wrappedGraphIndex] = currFPS;
  globalDebugData.wrappedGraphIndex = (globalDebugData.wrappedGraphIndex + 1) % DEBUG_TIMING_HISTORY_LENGTH;

  globalDebugData.weightedMS = globalDebugData.weightedMS * 0.95 + currMS * 0.05;
  globalDebugData.weightedFPS = globalDebugData.weightedFPS * 0.95 + currFPS * 0.05;
}

/*     GL Timers     */
void initGLTimer(GLTimer* timer)
{
  glGenQueries(1, &timer->query);
  errCheck();
}
void startGLTimer(GLTimer* timer)
{
  return;
  glBeginQuery(GL_TIME_ELAPSED, timer->query);
  errCheck();
}
void endGLTimer(GLTimer* timer)
{
  return;
  glEndQuery(GL_TIME_ELAPSED);
  errCheck();
}
int queryGLTimerReady(GLTimer* timer)
{
  int result;
  glGetQueryObjectiv(	timer->query,
			GL_QUERY_RESULT_AVAILABLE,
&result);
  errCheck();
  return result;

}
f32 getGLTimerResult(GLTimer* timer)
{
  u64 result;
  glGetQueryObjectui64v(timer->query,
		      GL_QUERY_RESULT,
		      &result);
  timer->elapsedMS = result / 1000000.0f;
  errCheck();
  return timer->elapsedMS;
}
void deleteGLTimer(GLTimer* timer)
{
  glDeleteQueries(1, &timer->query);
  errCheck();
}

bool isHoveringDebugConsole()
{
  return globalDebugData.ImGuiIo->WantCaptureMouse;
}

void drawImGuiXYZSlider(vec3* vec, f32 min, f32 max)
{
  ImGui::PushID(vec);
  ImGui::SliderFloat3("", &vec->x, min, max);
  ImGui::PopID();
}

void drawImGuiVec3Text(vec3 vec, const char* prefix)
{
  ImGui::Text("%s: %f %f %f", prefix, vec.x, vec.y, vec.z);
}

void drawDebugLightConsole()
{
  //Lights
  if (ImGui::CollapsingHeader("Lights"))
    {
      ImGui::Indent();
      
      //Point Lights
      if (ImGui::CollapsingHeader("Point Lights"))
	{
	  ImGui::PushID("Point Lights");
	  for (int i = 0; i < RENDERER_POINT_LIGHT_COUNT; i++)
	    {
	      ImGui::Text("Light: %d\n", i);
	      ImGui::PushID(i);           // Push i to the id stack (otherwise stuff affects the same thing
	      PointLight* p = &globalRenderData.pointLights[i];
	      {
		if(ImGui::CollapsingHeader("Position"))
		  {
		    drawImGuiXYZSlider(&p->position, -10.0f, 10.0f);
		  }
		if(ImGui::CollapsingHeader("Colour"))
		  {
		    ImGui::ColorPicker3("MyColor##4", (float*)&p->diffuseColour, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHex);
		  }
		ImGui::SliderFloat("Intensity: ", &p->intensity, 0.0f, 100.0f);
	      }
	      ImGui::PopID();
	    }
	  ImGui::PopID();

	}
            //Directional lights
      if (ImGui::CollapsingHeader("Directional Lights"))
	{
	  ImGui::PushID("Directional Lights");
	  for (int i = 0; i < RENDERER_DIRECTIONAL_LIGHT_COUNT; i++)
	    {
	      ImGui::Text("Dir Light: %d\n", i);
	      ImGui::PushID(i);           // Push i to the id stack (otherwise stuff affects the same thing
	      DirectionalLight* p = &globalRenderData.dirLights[i];
	      {
		if(ImGui::CollapsingHeader("Direction"))
		  {
		    drawImGuiXYZSlider(&p->direction, -1.0, 1.0);
		  }
		if(ImGui::CollapsingHeader("Colour"))
		  {
		    drawImGuiXYZSlider(&p->diffuseColour, 0.0, 1.0);
		  }
	      }
	           {
		// Using a Child allow to fill all the space of the window.
		// It also alows customization
		ImGui::BeginChild("GameRender");
		// Get the size of the child (i.e. the whole draw size of the windows).
		ImVec2 wsize = ImGui::GetWindowSize();
		// Because I use the texture from OpenGL, I need to invert the V from the UV.
		ImGui::Image((ImTextureID)globalRenderData.shadowMapFBO.textureKey, wsize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndChild();
		}
	      ImGui::PopID();
	    }
	  ImGui::PopID();
	}
      ImGui::Unindent();	     
    }
}


void drawDebugEntitiesConsole()
{

  if (ImGui::CollapsingHeader("Entities"))
  //  ImGui::Begin("Entities");
    {
      ImGui::Text("Total Entities: %d", globalEntityRegistry->entityCount);
      ImGui::Checkbox("Show AABBs", (bool*)&globalDebugData.showAABB);
      ImGui::Indent();
      for (int i = 0; i < MAX_REGISTRY_SIZE; i++)
	{
	  if (globalEntityRegistry->occupiedIndices[i] && globalEntityRegistry->entities[i].meshes)
	    {
	      Entity* e = globalEntityRegistry->entities + i;
	      ImGui::PushID(e->id);
	      /*if (e->id == globalDebugData.selectedEntityId)
		{
		  ImGui::SetNextTreeNodeOpen(true);
		  }*/
	      if (ImGui::CollapsingHeader(e->name))
		{
		  drawImGuiXYZSlider(&e->position, -10.0, 10.0);
		  drawImGuiXYZSlider(&e->scale, 0.0, 3.0);
		  drawImGuiVec3Text(e->position, "Pos");
		  int indexCount = 0;
		  int vertexCount = 0;
		  for (int m = 0; m < e->meshCount; m++)
		    {
		      indexCount += e->meshes[m]->indexCount;
		      vertexCount += e->meshes[m]->vertexCount;
		    }
		    
		  ImGui::Text("Vertex Count: %d", vertexCount);
		  ImGui::Text("Index Count: %d", indexCount);
		  ImGui::Checkbox("Visible", (bool*)&e->visible);
		  ImGui::Indent();
		  if (ImGui::CollapsingHeader("mesh debug info"))
		    {
		      for (int m = 0; m < e->meshCount; m++)
			{
			  ImGui::PushID(m);
			  ImGui::Text("indexCount: %d",e->meshes[m]->indexCount);
			  ImGui::Text("vertCount: %d",e->meshes[m]->vertexCount);
			  ImGui::Checkbox("Visible", (bool*)&e->meshes[m]->visible);
			  ImGui::PopID();
			}
		    }
		  if (ImGui::CollapsingHeader("PhysicsInfo"))
		    {
		      ImGui::Checkbox("Gravity", (bool*)&e->gravityEnabled);
		      ImGui::Checkbox("Physics Enabled", (bool*)&e->physicsEnabled);
	      
		      drawImGuiVec3Text(e->position, "Position");
		      drawImGuiVec3Text(e->rotation, "Rotation");
		      drawImGuiVec3Text(e->scale, "Scale");
		      drawImGuiVec3Text(e->velocity, "Velocity");
		    }
		  if (ImGui::CollapsingHeader("CollisionInfo"))
		    {
		      ImGui::Text("AABB");
		      drawImGuiVec3Text(e->collider.aabb.min, "min");
		      drawImGuiVec3Text(e->collider.aabb.max, "max");
		    }

		  ImGui::Unindent();
		}
	      ImGui::PopID();
	    }
	}
      ImGui::Unindent();
    }
}

void drawDebugFileInspector()
{
  ImGui::Begin("Files");

  if (strcmp(globalDebugData.currentFolder,"res/entities") != 0)
    {
      globalDebugData.currentFolderNames = getFileNamesFromPath("res/entities", &globalDebugData.currentFolderFileCount);
      strcpy(globalDebugData.currentFolder, "res/entities");

    }  
  for (int i = 0; i < globalDebugData.currentFolderFileCount; i++)
    {
      ImGui::PushID(i);
      if (i % 5 != 0)
	ImGui::SameLine();
      ImGui::Button(globalDebugData.currentFolderNames[i], ImVec2(60,60));
      
      //Start drag
      if (globalDebugData.ImGuiIo->WantCaptureMouse)
	{
	  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	    {
	      ImGui::SetDragDropPayload("DND_DEMO_CELL", globalDebugData.currentFolderNames[i], strlen(globalDebugData.currentFolderNames[i]) + 1);
	      ImGui::Text("Copy %s", globalDebugData.currentFolderNames[i]);
	      ImGui::EndDragDropSource();
	    }
	  if (ImGui::IsMouseDragging(0))
	    {
	      globalDebugData.wasMouseDragged = true;
	    }
	}
      //End Drag
      if (!globalDebugData.ImGuiIo->WantCaptureMouse)
	{
	  if (globalDebugData.wasMouseDragged && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	    {
	      printf("Copied!!!\n\n\n");
	      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
		{
		  printf("%s\n", payload->Data);
		  char buffer[256];
		  sprintf(buffer, "res/entities/%s", payload->Data);
		  Entity* entity = deserializeEntity(buffer);
		  entity->position = screenSelectMaxDist( pollCursorPos(), 1.0f);
		}
	      globalDebugData.wasMouseDragged = false;
	    }
	}
      ImGui::PopID();
    }
  
  ImGui::End();
}

void drawDebugMainMenuBar()
{
   if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
	  ImGui::MenuItem("(demo menu)", NULL, false, false);
	  if (ImGui::MenuItem("Open", "Ctrl+O")) {}
	  if (ImGui::BeginMenu("Open Recent"))
	    {
	      ImGui::MenuItem("fish_hat.c");
	      ImGui::MenuItem("fish_hat.inl");
	      ImGui::MenuItem("fish_hat.h");
	      if (ImGui::BeginMenu("More.."))
		{
		  ImGui::MenuItem("Hello");
		  ImGui::MenuItem("Sailor");
		  ImGui::EndMenu();
		}
	      ImGui::EndMenu();
	    }
	  if (ImGui::MenuItem("Save", "Ctrl+S")) {}
	  if (ImGui::MenuItem("Save As..")) {}

	  ImGui::Separator();

	  ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    } 
}

void drawMainDebugMenu()
{

  
  ImGui::Text("Frame: %d, %.2f%%", anim->currentFrames[0], anim->currentInterps[0]);
  ImGui::Text("Frame: %d, %.2f%%", anim->currentFrames[1], anim->currentInterps[1]);
  
  //Timing data
  updateGeneralDebugInfo();
  if (ImGui::CollapsingHeader("TimingData"))
    {

      f32 rawDeltaTime = globalDeltaTime / globalTimeScale;

      if (ImPlot::BeginPlot("FPS", ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Timestep","FPS", ImPlotAxisFlags_Lock, ImPlotAxisFlags_AutoFit );
	ImPlot::SetupAxesLimits(0,DEBUG_TIMING_HISTORY_LENGTH,0,120);
	ImPlot::PlotLine("FPS", globalDebugData.xAxis, globalDebugData.fpsHistory, DEBUG_TIMING_HISTORY_LENGTH);
        ImPlot::EndPlot();
      }
      if (ImPlot::BeginPlot("MS", ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Timestep","MS", ImPlotAxisFlags_Lock, ImPlotAxisFlags_AutoFit );
	ImPlot::SetupAxesLimits(0,DEBUG_TIMING_HISTORY_LENGTH,0,120);
	ImPlot::PlotLine("FPS", globalDebugData.xAxis, globalDebugData.msHistory, DEBUG_TIMING_HISTORY_LENGTH);
        ImPlot::EndPlot();
      }
      
      ImGui::Text("Weighted MS: %.0f", globalDebugData.weightedMS);
      ImGui::Text("Weighted FPS: %f", globalDebugData.weightedFPS);
      
      ImGui::Text("Delta Time Scale: %f", globalTimeScale);
      ImGui::Text("Raw Delta Time: %f", rawDeltaTime);
      ImGui::Text("Raw FPS: %.0f", 1 / rawDeltaTime);
      
      ImGui::Text("Total GPU time: %.3fms", GPUTotalTime);
      ImGui::Text("Mesh GPU time: %.3fms", GPUMeshTimer.elapsedMS);
      ImGui::Text("UI GPU time: %.3fms", GPUUITimer.elapsedMS);
      ImGui::Text("ImGUI GPU time: %.3fms", GPUImGUITimer.elapsedMS);
    }

  //Camera Info
  if (ImGui::CollapsingHeader("Camera"))
    {
      vec3 pos = getCameraPos(&mainCamera);
      drawImGuiVec3Text(pos, "Camera Position");
    }

  //Rendererinfo
  if (ImGui::CollapsingHeader("Renderer"))
    {
      //ImGui::SliderFloat("Bloom Strength: %f", &globalRenderData.bloomInfo.strength, 0.0f, 1.0f);
      //ImGui::SliderFloat("Bloom Cutoff: %f", &globalRenderData.bloomInfo.cutoff, 0.0f, 1.0f);
      ImGui::Text("Viewport: %d %d", globalRenderData.viewportWidth, globalRenderData.viewportHeight);
      ImGui::Text("outputFBO: %d %d", globalRenderData.outputFBO.width, globalRenderData.outputFBO.height);
      //Wireframe
      ImGui::Checkbox("Wireframes" , &globalRenderData.wireFrameMode);
      //ImGui::Checkbox("Palettize" , &globalRenderData.palettize);

      //PostProcessing values
      /*ImGui::ColorEdit4("Average Color:", &globalRenderData.averageColour.x);
      ImGui::Text("Exposure: %f", globalRenderData.exposure);
      ImGui::SliderFloat("Exposure Rate: %f", &globalRenderData.exposureChangeRate, 0.0f, 10.0f);
      if(ImGui::ListBoxHeader("PostProcessingShaders"))
	{
	  for (int i = 0; i < 5; i++)
	    {
	      char item[3];
	      sprintf(item, "%d", i);
	      if (ImGui::Selectable(item, globalRenderData.enabledScreenShader == i))
		{
		  globalRenderData.enabledScreenShader = i;
		  setFrameBufferShader(i);
		}
	    }
	  ImGui::ListBoxFooter();
	}
      */
      //Vsync
      if (ImGui::Checkbox("Toggle Vsync", (bool*)&mainWindow.vSyncOn))
	setVSync(mainWindow.vSyncOn);

    }

  //UI Text values
  if (ImGui::CollapsingHeader("Text"))
    {				     
      ImGui::SliderFloat("Kerning", &globalKerning, 0.0f, 1.0f);
      ImGui::SliderFloat("Size", &fontSize, 0.0f, 720.0f);
    }

  drawDebugLightConsole();
  drawDebugEntitiesConsole();
      
  
}

void drawDebugEntityInspector()
{
  Entity* e = getEntityById(globalDebugData.selectedEntityId);
  if (!e) return;
  ImGui::PushID(e->id);
  ImGui::Text("Name: %s", e->name);
  if(e->parent)
    {
      if (ImGui::Button("Select Parent"))
	{
	  ImGui::PopID();
	  globalDebugData.selectedEntityId = e->parent->id;
	  return;
	}
    }
  char header[64];
  sprintf(header, "Children (%d)", e->childCount);
  if(ImGui::CollapsingHeader(header))
    {
      for (int i = 0; i < e->childCount; i++)
	{
	  ImGui::PushID(e->children[i]->id);
	  if(ImGui::Button(e->children[i]->name))
	    {
	      ImGui::PopID();
	      ImGui::PopID();
	      globalDebugData.selectedEntityId = e->children[i]->id;
	      return;
	    }
	  ImGui::PopID();
	}
    }
  ImGui::Text("Position:");
  drawImGuiXYZSlider(&e->position, -10.0, 10.0);
  ImGui::Text("Scale:");
  drawImGuiXYZSlider(&e->scale, 0.0, 3.0);
  ImGui::Text("Rotation:");
  drawImGuiXYZSlider(&e->rotation, 0.0, 360.0);

  int indexCount = 0;
  int vertexCount = 0;
  for (int m = 0; m < e->meshCount; m++)
    {
      indexCount += e->meshes[m]->indexCount;
      vertexCount += e->meshes[m]->vertexCount;
    }
		    
  ImGui::Text("Vertex Count: %d", vertexCount);
  ImGui::Text("Index Count: %d", indexCount);
  ImGui::Checkbox("Visible", (bool*)&e->visible);
  ImGui::Indent();
  if (ImGui::CollapsingHeader("mesh debug info"))
    {
      for (int m = 0; m < e->meshCount; m++)
	{
	  ImGui::PushID(m);
	  ImGui::Text("indexCount: %d",e->meshes[m]->indexCount);
	  ImGui::Text("vertCount: %d",e->meshes[m]->vertexCount);
	  ImGui::Checkbox("Visible", (bool*)&e->meshes[m]->visible);
	  ImGui::PopID();
	}
    }
  if (ImGui::CollapsingHeader("PhysicsInfo"))
    {
      ImGui::Checkbox("Gravity", (bool*)&e->gravityEnabled);
      ImGui::Checkbox("Physics Enabled", (bool*)&e->physicsEnabled);
	      
      drawImGuiVec3Text(e->position, "Position");
      drawImGuiVec3Text(e->rotation, "Rotation");
      drawImGuiVec3Text(e->scale, "Scale");
      drawImGuiVec3Text(e->velocity, "Velocity");
    }
  if (ImGui::CollapsingHeader("CollisionInfo"))
    {
      ImGui::Text("AABB");
      drawImGuiVec3Text(e->collider.aabb.min, "min");
      drawImGuiVec3Text(e->collider.aabb.max, "max");
    }

  ImGui::Unindent();
  ImGui::PopID();
}
  


/*   Debug Console     */
void drawDebugConsole()
{
  for (int i = 0; i < RENDERER_POINT_LIGHT_COUNT; i++)
    {
      Entity* gizmo = getEntityById(globalRenderData.pointLights[i].entityGizmoID);
      for (int j = 0; j < gizmo->meshCount; j++)
	{
	  /*drawMesh( gizmo->meshes[j],
		    globalRenderData.pointLights[i].position,
		    gizmo->rotation,
		    gizmo->scale );*/
	}
    }

  glClear(GL_DEPTH_BUFFER_BIT);  
  
  //Anything that should be drawn ontop of everything
  /*Entity* arrows = getEntityById(translationArrows);
  for (int i = 0; i < arrows->meshCount; i++)
    {
      //drawMesh(arrows->meshes[i], arrows->position, arrows->rotation, arrows->scale);
      }*/


    
  ImGui::NewFrame();
  ImGui::ShowDemoWindow();
  ImGui::Begin("Editor");
  if(ImGui::BeginTabBar("Tabs"))
    {
      if(ImGui::BeginTabItem("Menu"))
	{
	  drawMainDebugMenu();
	  ImGui::EndTabItem();
	}
      if(ImGui::BeginTabItem("Entity Inspector"))
	{
	  drawDebugEntityInspector();
	  ImGui::EndTabItem();
	}
      ImGui::EndTabBar();
    }
  ImGui::End();
  drawDebugMainMenuBar();  
  drawDebugFileInspector();
  
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


}
