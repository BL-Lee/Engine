#include "DebugConsole.h"


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



void drawDebugConsole()
{
    //DEBUG CONSOLE
  //Anything that should be drawn ontop
  
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

  Entity* arrows = getEntityById(translationArrows);
  for (int i = 0; i < arrows->meshCount; i++)
    {
      //drawMesh(arrows->meshes[i], arrows->position, arrows->rotation, arrows->scale);
    }
    
  ImGui::NewFrame();
  ImGui::Begin("Editor");

  if (ImGui::CollapsingHeader("TimingData"))
    {
      ImGui::Text("Delta Time Scale: %f", globalTimeScale);
      ImGui::Text("Delta Time: %f", globalDeltaTime);
      ImGui::Text("Total GPU time: %.3fms", GPUTotalTime);
      ImGui::Text("Mesh GPU time: %.3fms", GPUMeshTimer.elapsedMS);
      ImGui::Text("UI GPU time: %.3fms", GPUUITimer.elapsedMS);
      ImGui::Text("ImGUI GPU time: %.3fms", GPUImGUITimer.elapsedMS);
    }
      
  if (ImGui::CollapsingHeader("Camera"))
    {
      vec3 pos = getCameraPos(&mainCamera);
      ImGui::Text("Camera Position: x: %f y: %f z: %f", pos.x, pos.y, pos.z);             
    }

  if (ImGui::CollapsingHeader("Renderer"))
    {
      ImGui::Checkbox("Wireframes" , &globalRenderData.wireFrameMode);
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
      ImGui::ColorEdit4("Average Color:", &globalRenderData.averageColour.x);
      ImGui::Text("Exposure: %f", globalRenderData.exposure);
      ImGui::SliderFloat("Exposure Rate: %f", &globalRenderData.exposureChangeRate, 0.0f, 10.0f);
      ImGui::ListBoxHeader("PostProcessingShaders");
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

      if (ImGui::Checkbox("Toggle Vsync", (bool*)&mainWindow.vSyncOn))
	setVSync(mainWindow.vSyncOn);
    }
  if (ImGui::CollapsingHeader("Lights"))
    {
      ImGui::Indent();
      if (ImGui::CollapsingHeader("Point Lights"))
	{
	  for (int i = 0; i < RENDERER_POINT_LIGHT_COUNT; i++)
	    {
	      ImGui::Text("Light: %d\n", i);
	      ImGui::PushID(i);           // Push i to the id stack (otherwise stuff affects the same thing
	      PointLight* p = &globalRenderData.pointLights[i];
	      {
		if(ImGui::CollapsingHeader("Position"))
		  {
		    ImGui::SliderFloat("X: ", &p->position.x, -10.0, 10.0f);
		    ImGui::SliderFloat("Y: ", &p->position.y, -10.0, 10.0f);
		    ImGui::SliderFloat("Z: ", &p->position.z, -10.0, 10.0f);
		  }
		if(ImGui::CollapsingHeader("Colour"))
		  {
		    ImGui::SliderFloat("R: ", &p->diffuseColour.x, 0.0, 1.0f);
		    ImGui::SliderFloat("G: ", &p->diffuseColour.y, 0.0, 1.0f);
		    ImGui::SliderFloat("B: ", &p->diffuseColour.z, 0.0, 1.0f);
		  }
		ImGui::SliderFloat("Intensity: ", &p->intensity, 0.0f, 10.0f);
	      }
	      ImGui::PopID();
	    }

	}

      if (ImGui::CollapsingHeader("Directional Lights"))
	{
	  for (int i = 0; i < RENDERER_DIRECTIONAL_LIGHT_COUNT; i++)
	    {
	      ImGui::Text("Dir Light: %d\n", i);
	      ImGui::PushID(i);           // Push i to the id stack (otherwise stuff affects the same thing
	      DirectionalLight* p = &globalRenderData.dirLights[i];
	      {
		if(ImGui::CollapsingHeader("Direction"))
		  {
		    ImGui::SliderFloat("X: ", &p->direction.x, -1.0, 1.0f);
		    ImGui::SliderFloat("Y: ", &p->direction.y, -1.0, 1.0f);
		    ImGui::SliderFloat("Z: ", &p->direction.z, -1.0, 1.0f);
		  }
		if(ImGui::CollapsingHeader("Colour"))
		  {
		    ImGui::SliderFloat("R: ", &p->diffuseColour.x, 0.0, 1.0f);
		    ImGui::SliderFloat("G: ", &p->diffuseColour.y, 0.0, 1.0f);
		    ImGui::SliderFloat("B: ", &p->diffuseColour.z, 0.0, 1.0f);
		  }
	      }
	      ImGui::PopID();
	    }
	}
      ImGui::Unindent();	  

    }

  if (ImGui::CollapsingHeader("Text"))
    {				     
      ImGui::SliderFloat("Kerning", &globalKerning, 0.0f, 1.0f);
      ImGui::SliderFloat("Size", &fontSize, 0.0f, 720.0f);
    } 
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
		}
	      ImGui::PopID();
	    }
	}
      ImGui::Unindent();
    }
      
  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}
