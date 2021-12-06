

void drawDebugConsole()
{
    //DEBUG CONSOLE
  //Anything that should be drawn ontop
  glClear(GL_DEPTH_BUFFER_BIT);
  Entity* arrows = getEntityById(translationArrows);
  for (int i = 0; i < arrows->meshCount; i++)
    {
      drawMesh(arrows->meshes[i], arrows->position, arrows->rotation, arrows->scale);
    }
    
  ImGui::NewFrame();
  ImGui::Begin("Editor");

  if (ImGui::CollapsingHeader("TimingData"))
    {
      ImGui::Text("Delta Time Scale: %f", globalTimeScale);
      ImGui::Text("Delta Time: %f", globalDeltaTime);
    }
      
  if (ImGui::CollapsingHeader("Camera"))
    {
      //	  ImGui::Text("Camera Position: x: %f y: %f z: %f", mainCamera.pos.x, mainCamera.pos.y, mainCamera.pos.z);
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
    }

  ImGui::SliderFloat("Kerning", &globalKerning, 0.0f, 1.0f);
  ImGui::SliderFloat("Size", &fontSize, 0.0f, 720.0f);
      
  ImGui::End();
    
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}
