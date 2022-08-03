#include "DebugConsole.h"


/*     GL Timers     */
void initGLTimer(GLTimer* timer)
{
  glGenQueries(1, &timer->query);
  errCheck();
}
void startGLTimer(GLTimer* timer)
{
  glBeginQuery(GL_TIME_ELAPSED, timer->query);
  errCheck();
}
void endGLTimer(GLTimer* timer)
{
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

void drawImGuiRGBSlider(vec3* vec, f32 min, f32 max)
{
  ImGui::SliderFloat("R: ", &vec->x, min, max);
  ImGui::SliderFloat("G: ", &vec->y, min, max);
  ImGui::SliderFloat("B: ", &vec->z, min, max);  
}
void drawImGuiXYZSlider(vec3* vec, f32 min, f32 max)
{
  ImGui::SliderFloat("X: ", &vec->x, min, max);
  ImGui::SliderFloat("Y: ", &vec->y, min, max);
  ImGui::SliderFloat("Z: ", &vec->z, min, max);  
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
		    drawImGuiRGBSlider(&p->diffuseColour, 0.0f, 1.0f);
		  }
		ImGui::SliderFloat("Intensity: ", &p->intensity, 0.0f, 10.0f);
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
		    drawImGuiRGBSlider(&p->diffuseColour, 0.0, 1.0);
		  }
	      }
	      {
		// Using a Child allow to fill all the space of the window.
		// It also alows customization
		ImGui::BeginChild("GameRender");
		// Get the size of the child (i.e. the whole draw size of the windows).
		ImVec2 wsize = ImGui::GetWindowSize();
		// Because I use the texture from OpenGL, I need to invert the V from the UV.
		ImGui::Image((ImTextureID)globalRenderData.shadowMapTexture, wsize, ImVec2(0, 1), ImVec2(1, 0));
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
  u32 entityIndex = 0;
    //Entities
  //TODO: put in separate window
  if (ImGui::CollapsingHeader("Entity"))
    {
      ImGui::Indent();
      if (globalEntityRegistry->occupiedIndices[entityIndex] && globalEntityRegistry->entities[entityIndex].meshes)
	{
	  Entity* e = globalEntityRegistry->entities + entityIndex;
	  ImGui::PushID(e->id);	  
	  ImGui::Text("Pos: %f %f %f", e->position.x, e->position.y, e->position.z);
	  int indexCount = 0;
	  int vertexCount = 0;
	  for (int m = 0; m < e->meshCount; m++)
	    {
	      indexCount += e->meshes[m]->rendererData.indexCount;
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
		  ImGui::Text("indexCount: %d",e->meshes[m]->rendererData.indexCount);
		  ImGui::Text("vertCount: %d",e->meshes[m]->vertexCount);
		  ImGui::Checkbox("Visible", (bool*)&e->meshes[m]->visible);
		  ImGui::PopID();
		}
	    }
	  if (ImGui::CollapsingHeader("PhysicsInfo"))
	    {
	      drawImGuiVec3Text(e->position, "Position");
	      drawImGuiVec3Text(e->rotation, "Rotation");
	      drawImGuiVec3Text(e->scale, "Scale");
	      drawImGuiVec3Text(e->velocity, "Velocity");
	    }

	  ImGui::Unindent();
	  ImGui::PopID();
	}
    }
  ImGui::Unindent();
}

void drawDebugEntityConsole()
{
  if (ImGui::CollapsingHeader("Entities"))
    {
      ImGui::Indent();
      for (int i = 0; i < MAX_REGISTRY_SIZE; i++)
	{
	  if (globalEntityRegistry->occupiedIndices[i] && globalEntityRegistry->entities[i].meshes)
	    {
	      Entity* e = globalEntityRegistry->entities + i;
	      ImGui::PushID(e->id);
	      if (ImGui::CollapsingHeader(e->name))
		{
		  ImGui::Text("Pos: %f %f %f", e->position.x, e->position.y, e->position.z);
		  int indexCount = 0;
		  int vertexCount = 0;
		  for (int m = 0; m < e->meshCount; m++)
		    {
		      indexCount += e->meshes[m]->rendererData.indexCount;
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
			  ImGui::Text("indexCount: %d",e->meshes[m]->rendererData.indexCount);
			  ImGui::Text("vertCount: %d",e->meshes[m]->vertexCount);
			  ImGui::Checkbox("Visible", (bool*)&e->meshes[m]->visible);
			  ImGui::PopID();
			}
		    }
		  if (ImGui::CollapsingHeader("PhysicsInfo"))
		    {
		      drawImGuiVec3Text(e->position, "Position");
		      drawImGuiVec3Text(e->rotation, "Rotation");
		      drawImGuiVec3Text(e->scale, "Scale");
		      drawImGuiVec3Text(e->velocity, "Velocity");
		    }

		  ImGui::Unindent();
		}
	      ImGui::PopID();
	    }
	}
      ImGui::Unindent();
    }
}  


/*   Debug Console     */
void drawDebugConsole()
{
  for (int i = 0; i < RENDERER_POINT_LIGHT_COUNT; i++)
    {
      Entity* gizmo = getEntityById(globalRenderData.pointLights[i].entityGizmoID);
      for (int j = 0; j < gizmo->meshCount; j++)
	{
	  drawMesh( gizmo->meshes[j],
		    globalRenderData.pointLights[i].position,
		    gizmo->rotation,
		    gizmo->scale );
	}
    }

  glClear(GL_DEPTH_BUFFER_BIT);
  
  //Anything that should be drawn ontop of everything
  Entity* arrows = getEntityById(translationArrows);
  for (int i = 0; i < arrows->meshCount; i++)
    {
      //drawMesh(arrows->meshes[i], arrows->position, arrows->rotation, arrows->scale);
    }
    
  ImGui::NewFrame();
  ImGui::Begin("Editor");

  ImGui::Text("Frame: %d, %.2f%%", anim->currentFrames[0], anim->currentInterps[0]);
      ImGui::Text("Frame: %d, %.2f%%", anim->currentFrames[1], anim->currentInterps[1]);
  //Timing data
  if (ImGui::CollapsingHeader("TimingData"))
    {
      ImGui::Text("Delta Time Scale: %f", globalTimeScale);
      ImGui::Text("Delta Time: %f", globalDeltaTime);
      ImGui::Text("fps: %.0f", 1 / globalDeltaTime);
      ImGui::Text("Total GPU time: %.3fms", GPUTotalTime);
      ImGui::Text("Mesh GPU time: %.3fms", GPUMeshTimer.elapsedMS);
      ImGui::Text("UI GPU time: %.3fms", GPUUITimer.elapsedMS);
      ImGui::Text("ImGUI GPU time: %.3fms", GPUImGUITimer.elapsedMS);
    }

  //Camera Info
  if (ImGui::CollapsingHeader("Camera"))
    {
      vec3 pos = getCameraPos(&mainCamera);
      ImGui::Text("Camera Position: x: %f y: %f z: %f", pos.x, pos.y, pos.z);             
    }

  //Renderer info
  if (ImGui::CollapsingHeader("Renderer"))
    {
      //Wireframe
      ImGui::Checkbox("Wireframes" , &globalRenderData.wireFrameMode);
      //Index counts for renderer buffers
      for (int i = 0; i < RENDERER_BUFFER_COUNT; i++)
	{
	  switch(i)
	    {
	    case RENDERER_UI_MODE:
	      ImGui::Text("UI");
	      break;
	    case RENDERER_TEXTURE_MODE:
	      ImGui::Text("Textured");
	      break;
	    }
	  ImGui::Text("Index Count: %d", globalRenderData._indexCount[i]);
	}

      //PostProcessing values
      ImGui::ColorEdit4("Average Color:", &globalRenderData.averageColour.x);
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
      
  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}
