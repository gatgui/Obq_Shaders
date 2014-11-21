/*
Obq_StereoLens.cpp :

Obq_StereoLens is a lens shader that renders both left and right cameras 
of a stereo rig in the same frame, saving you one scene loading time for
each pair of stereo frame. This shader is applied on the center camera.

*------------------------------------------------------------------------
Copyright (c) 2012-2014 Marc-Antoine Desjardins, ObliqueFX (madesjardins@obliquefx.com)

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
*------------------------------------------------------------------------
*/

#include "Obq_StereoLens.h"


// Parameters
//
node_parameters
{
	AiParameterINT("viewMode" , 0);
	AiParameterSTR("leftCamera" , "Camera_L");
	AiParameterSTR("rightCamera" , "Camera_R");
	AiParameterFLT("filmbackX" , 0.0f);
	AiParameterFLT("leftCenterOffset" , 0.0f);
	AiParameterFLT("rightCenterOffset" , 0.0f);
	AiParameterBOOL("recalculateDistanceForSideCameras",true);
	AiParameterINT("totalOverscanPixels",0);
	
	AiParameterBOOL("useDof",false);
	AiParameterFLT("focusDistance",100.0f);
	AiParameterFLT("apertureSize",0.1f);
	AiParameterFLT("apertureAspectRatio",1.0f);
	AiParameterBOOL("usePolygonalAperture",true);
	AiParameterINT("apertureBlades",5);
	AiParameterFLT("apertureBladeCurvature",0.0f);
	AiParameterFLT("apertureRotation",0.0f);
	AiParameterBOOL("focusPlaneIsPlane",true);
	AiParameterBOOL("bokehInvert",false);
	AiParameterFLT("bokehBias",0.5f);
	AiParameterFLT("bokehGain",0.5f);


	AtArray* a = AiArray(2, 1, AI_TYPE_FLOAT, 0.0f,0.0f);
	AiParameterARRAY("fov",a);
	
}


node_initialize
{
	ShaderData *data = (ShaderData*) AiMalloc(sizeof(ShaderData));
	data->aspect = 1.0f;

	// Initialize
	data->aspect = 1.0f;
	data->width = 1920.0f;
	data->height = 1080.0f;
	data->pixelRatio = 1.0f;
	data->leftCamera = NULL;
	data->rightCamera = NULL;
	data->centerCamera = NULL;
	data->leftCenterOffset = 0.0f;
	data->rightCenterOffset = 0.0f;
	data->tan_myFov = 1.0f;
	data->useDof = false;
	data->apertureSize = 0.1f;
	data->apertureAspectRatio = 1.0f;
	data->apertureBlades = 5;
	data->apertureBladeCurvature = 0.0f;
	data->apertureRotation = 0.0f;
	data->bokehInvert = false;
	data->bokehBias = 0.5f;
	data->bokehGain = 0.5f;
	data->focalDistanceC = 100.0f;
	data->focalDistanceL = 100.0f;
	data->focalDistanceR = 100.0f;
	data->overscanRatio = 1.0f;
	data->viewMode = THISCAMERA;
	data->usePolygonalAperture = true;
	data->focusDistance = 100.0f;
	data->focusPlaneIsPlane = true;

	// Set data
	AiCameraInitialize(node, data);
}

node_update
{
	AiCameraUpdate(node, false);

	ShaderData *data = (ShaderData*)AiCameraGetLocalData(node);

	// Update shaderData variables
	AtNode* options = AiUniverseGetOptions();
	
	// Overscan
	float w = float(AiNodeGetInt(options,"xres"));
	float h = float(AiNodeGetInt(options,"yres"));
	
	switch(params[p_viewMode].INT)
	{
	case STEREOLR:
		data->width  = w - params[p_totalOverscanPixels].INT;
		data->height  = h;
		data->overscanRatio = w/data->width;
		break;
	case STEREODU:
		data->width  = w;
		data->height  = h - params[p_totalOverscanPixels].INT;
		data->overscanRatio = h/data->height;
		break;
	default:
		data->width  = w;
		data->height  = h;
		data->overscanRatio = 1.0f;
		break;
	}

	// Aspect
	data->pixelRatio = 1.0f/AiNodeGetFlt(options,"aspect_ratio");
	data->aspect = data->width/(data->height/data->pixelRatio);

	// Get all 3 cameras and all 3 matrices
	AtMatrix centerCameraMatrix,leftCameraMatrix,rightCameraMatrix;
	data->centerCamera = node;
	AiNodeGetMatrix(node,"matrix",centerCameraMatrix);
	std::string camNodeName(AiNodeGetName(node));
	std::size_t sitoaIndex = camNodeName.rfind(".SItoA.");

	data->leftCamera = AiNodeLookUpByName((std::string(params[p_leftCamera].STR).append(camNodeName.substr(sitoaIndex)).c_str()));
	AiNodeGetMatrix(data->leftCamera,"matrix",leftCameraMatrix);

	data->rightCamera = AiNodeLookUpByName((std::string(params[p_rightCamera].STR).append(camNodeName.substr(sitoaIndex)).c_str()));
	AiNodeGetMatrix(data->rightCamera,"matrix",rightCameraMatrix);

	data->viewMode = params[p_viewMode].INT;

	if(data->centerCamera==NULL)
	{
		AiMsgError("Error with center camera... this is not normal...");
	}
	else if( data->leftCamera==NULL)
	{
		AiMsgError("Left camera doesn't exist. Don't forget the Model name if any. ex: Model.LeftCamera");
	}
	else if( data->rightCamera==NULL)
	{
		AiMsgError("Right camera doesn't exist. Don't forget the Model name if any. ex: Model.RightCamera");
	}
	else
	{

		// Field of view
		float fov = AiArrayGetFlt(AiNodeGetArray(node, "fov"),0); // good default value from current camera but left or right should be 
		AtArray* aFov = NULL;
		aFov = AiNodeGetArray(data->leftCamera, "fov"); // Get horizontal angle fov from left camera
		if(aFov !=NULL)
		{
			fov = AiArrayGetFlt(aFov,0);
		}
		else
		{
			aFov = AiNodeGetArray(data->rightCamera, "fov"); // If left camera doesn't have one, get horizontal angle from right camera
			if(aFov !=NULL)
			{
				fov = AiArrayGetFlt(aFov,0);
			}
			else
				fov = AiArrayGetFlt(AiNodeGetArray(node, "fov"),0); // If neither left nor right, get fov from this camera (always good)
		}
		
		
		data->tan_myFov = std::tan(float(fov * AI_DTOR / 2.0));
		
		// Rotation and Translation Matrix
		AtMatrix iCenterCameraMatrix;
		AiM4Invert(centerCameraMatrix, iCenterCameraMatrix);
		AiM4Mult(data->center2leftCameraMatrix, leftCameraMatrix, iCenterCameraMatrix);
		AiM4Mult(data->center2rightCameraMatrix,rightCameraMatrix, iCenterCameraMatrix);

		// Optical Center Offsets
		data->leftCenterOffset  = 2.0f*params[p_leftCenterOffset].FLT/params[p_filmbackX].FLT;
		data->rightCenterOffset = 2.0f*params[p_rightCenterOffset].FLT/params[p_filmbackX].FLT;

		// DOF
		data->useDof = params[p_useDof].BOOL;
		if(data->useDof)
		{
			// DOF related values
			data->focalDistanceC = params[p_focusDistance].FLT;
			data->apertureSize = params[p_apertureSize].FLT;
			data->apertureAspectRatio = params[p_apertureAspectRatio].FLT;
			data->usePolygonalAperture = params[p_usePolygonalAperture].BOOL;
			data->apertureBlades = params[p_apertureBlades].INT;
			data->apertureBladeCurvature = params[p_apertureBladeCurvature].FLT;
			data->apertureRotation = params[p_apertureRotation].FLT;
			data->bokehInvert = params[p_bokehInvert].BOOL;
			data->bokehBias = params[p_bokehBias].FLT;
			data->bokehGain = 1.0f-params[p_bokehGain].FLT;

			if(data->apertureAspectRatio<=0.0f)
				data->apertureAspectRatio = 0.001f;

			data->focusPlaneIsPlane = params[p_focusPlaneIsPlane].BOOL;

			// Focal Plane
			if(params[p_recalculateDistanceForSideCameras].BOOL)
			{
				// calculate distance between center and left and between center and right
				float dLeft = std::sqrt(data->center2leftCameraMatrix[3][0]*data->center2leftCameraMatrix[3][0] + data->center2leftCameraMatrix[3][1]*data->center2leftCameraMatrix[3][1] + data->center2leftCameraMatrix[3][2]*data->center2leftCameraMatrix[3][2]);
				float dRight = std::sqrt(data->center2rightCameraMatrix[3][0]*data->center2rightCameraMatrix[3][0] + data->center2rightCameraMatrix[3][1]*data->center2rightCameraMatrix[3][1] + data->center2rightCameraMatrix[3][2]*data->center2rightCameraMatrix[3][2]);

				// Set the focal distances;
				data->focalDistanceL = std::sqrt(dLeft*dLeft + data->focalDistanceC*data->focalDistanceC);
				data->focalDistanceR = std::sqrt(dRight*dRight + data->focalDistanceC*data->focalDistanceC);
			}
			else
			{
				data->focalDistanceL = data->focalDistanceC;
				data->focalDistanceR = data->focalDistanceC;
			}
		}

		AiMsgInfo("----------------------------------------------------");
		AiMsgInfo("CameraParams : fov = %f",fov);
		AiMsgInfo("CameraParams : filmback width = %f",params[p_filmbackX].FLT);
		AiMsgInfo("CameraParams : left optical center offset = %f",params[p_leftCenterOffset].FLT);
		AiMsgInfo("CameraParams : right optical center offset = %f",params[p_rightCenterOffset].FLT);
		AiMsgInfo("CameraParams : overscan ratio = %f", data->overscanRatio);
		if(data->useDof)
		{
			AiMsgInfo("CameraParams : focal distance = %f",data->focalDistanceC);
			AiMsgInfo("CameraParams : focal distance from left   = %f",data->focalDistanceL);
			AiMsgInfo("CameraParams : focal distance from right  = %f",data->focalDistanceR);
			AiMsgInfo("CameraParams : aperture size = %f", data->apertureSize);
			AiMsgInfo("CameraParams : aperture ratio = %f", data->apertureAspectRatio);
			AiMsgInfo("CameraParams : use polygonal aperture = %s", (data->usePolygonalAperture?"True":"False"));
			AiMsgInfo("CameraParams : aperture blades = %i", data->apertureBlades);
			AiMsgInfo("CameraParams : aperture blade curvature = %f", data->apertureBladeCurvature);
			AiMsgInfo("CameraParams : aperture blade rotation = %f", data->apertureRotation);
		}
		AiMsgInfo("----------------------------------------------------");
	}

}

node_finish
{
	ShaderData *data = (ShaderData*)AiCameraGetLocalData(node);
	AiFree(data);
	AiCameraDestroy(node);
}

camera_create_ray 
{
	// AspectRatio
	ShaderData *data = (ShaderData*)AiCameraGetLocalData(node);
	float aspect = data->aspect;
	
	// Scale derivatives
	float dsx = input->dsx*data->tan_myFov;
	float dsy = input->dsy*data->tan_myFov;

	// ViewMode
	switch(data->viewMode)
	{
	case THISCAMERA:
		{
			// Direction and optical center offset 
			AtPoint p = { input->sx, input->sy, -1.0f/data->tan_myFov };
			output->dir = AiV3Normalize(p - output->origin);

			//DOF
			if(data->useDof && data->apertureSize >0.0f){
				float lensU;
				float lensV;
				ConcentricSampleDisk(input->lensx, input->lensy, data->apertureBlades, data->apertureBladeCurvature, data->apertureRotation,&lensU, &lensV, data->bokehInvert, data->bokehBias, data->bokehGain);
				lensU*=data->apertureSize;
				lensV*=data->apertureSize;
				float ft = (data->focusPlaneIsPlane?std::abs(data->focalDistanceC/output->dir.z):data->focalDistanceC);
				AtPoint Pfocus = output->dir*ft;

				// Focal Aspect Ratio
				lensV*=data->apertureAspectRatio;

				output->origin.x = lensU;
				output->origin.y = lensV;
				output->origin.z = 0.0;
				output->dir = AiV3Normalize(Pfocus-output->origin);
			}

			break;
		}
	case LEFTCAMERA:
		{
			// Direction and optical center offset 
			AtPoint p = { input->sx+data->leftCenterOffset , input->sy, -1.0f/data->tan_myFov };
			AtVector dir = AiV3Normalize(p - output->origin);
			bool outputDone = false;

			if(data->useDof && data->apertureSize >0.0f){
				float lensU;
				float lensV;
				ConcentricSampleDisk(input->lensx, input->lensy, data->apertureBlades, data->apertureBladeCurvature, data->apertureRotation,&lensU, &lensV, data->bokehInvert, data->bokehBias, data->bokehGain);
				lensU*=data->apertureSize;
				lensV*=data->apertureSize;

				float ft = data->focusPlaneIsPlane?std::abs(data->focalDistanceL/dir.z):data->focalDistanceL;
				AtPoint Pfocus = dir*ft;

				// Focal Aspect Ratio
				lensV*=data->apertureAspectRatio;

				output->origin.x = lensU;
				output->origin.y = lensV;
				output->origin.z = 0.0;
				dir = AiV3Normalize(Pfocus-output->origin);
			}

			// Check if output already done
			if(!outputDone)
			{
				// Rotate
				AtVector dir2;
				AiM4VectorByMatrixMult(&dir2,data->center2leftCameraMatrix,&dir);
				output->dir = AiV3Normalize(dir2);

				// Move origin
				const AtPoint po = {output->origin.x,output->origin.y,output->origin.z};
				AiM4PointByMatrixMult(&output->origin,data->center2leftCameraMatrix,&po);
			}
			break;
		}
	case RIGHTCAMERA:
		{
			// Direction and optical center offset 
			AtPoint p = { input->sx+data->rightCenterOffset , input->sy, -1.0f/data->tan_myFov };
			AtVector dir = AiV3Normalize(p - output->origin);
			bool outputDone = false;

			// DOF
			if(data->useDof && data->apertureSize >0.0f){
				float lensU;
				float lensV;
				ConcentricSampleDisk(input->lensx, input->lensy, data->apertureBlades, data->apertureBladeCurvature, data->apertureRotation,&lensU, &lensV, data->bokehInvert, data->bokehBias, data->bokehGain);
				lensU*=data->apertureSize;
				lensV*=data->apertureSize;
				float ft = data->focusPlaneIsPlane?std::abs(data->focalDistanceR/dir.z):data->focalDistanceR;
				AtPoint Pfocus = dir*ft;

				// Focal Aspect Ratio
				lensV*=data->apertureAspectRatio;

				output->origin.x = lensU;
				output->origin.y = lensV;
				output->origin.z = 0.0;
				dir = AiV3Normalize(Pfocus-output->origin);
			}

			if(!outputDone)
			{
				// Rotate
				AtVector dir2;
				AiM4VectorByMatrixMult(&dir2,data->center2rightCameraMatrix,&dir);
				output->dir = AiV3Normalize(dir2);

				// Move origin
				const AtPoint po = {output->origin.x,output->origin.y,output->origin.z};
				AiM4PointByMatrixMult(&output->origin,data->center2rightCameraMatrix,&po);
			}
			break;
		}
	case STEREOLR:
		{

			// Equal derivatives y is ok, x is bad
			dsx=dsy;

			// Left Camera
			if(input->sx < 0)
			{
				// Direction and optical center offset 
				AtPoint p = { (input->sx+0.5f)*2.0f*data->overscanRatio+data->leftCenterOffset , input->sy, -1.0f/data->tan_myFov };
				AtVector dir = AiV3Normalize(p - output->origin);
				bool outputDone = false;

				// DOF
				if(data->useDof && data->apertureSize >0.0f){
					float lensU;
					float lensV;
					ConcentricSampleDisk(input->lensx, input->lensy, data->apertureBlades, data->apertureBladeCurvature, data->apertureRotation, &lensU, &lensV, data->bokehInvert, data->bokehBias, data->bokehGain);
					lensU*=data->apertureSize;
					lensV*=data->apertureSize;
					float ft = data->focusPlaneIsPlane?std::abs(data->focalDistanceL/dir.z):data->focalDistanceL;
					AtPoint Pfocus = dir*ft;
					// Focal Aspect Ratio
					lensV*=data->apertureAspectRatio;

					output->origin.x = lensU;
					output->origin.y = lensV;
					output->origin.z = 0.0;
					dir = AiV3Normalize(Pfocus-output->origin);
				}

				if(!outputDone)
				{
					// Rotate
					AtVector dir2;
					AiM4VectorByMatrixMult(&dir2,data->center2leftCameraMatrix,&dir);
					output->dir = AiV3Normalize(dir2);

					// Move origin
					const AtPoint po = {output->origin.x,output->origin.y,output->origin.z};
					AiM4PointByMatrixMult(&output->origin,data->center2leftCameraMatrix,&po);
				}

				// derivatives and overscan
				//dsx*=data->overscanRatio;

			}
			else // Right camera
			{
				AtPoint p = { (input->sx-0.5f)*2.0f*data->overscanRatio+data->rightCenterOffset , input->sy, -1.0f/data->tan_myFov };
				AtVector dir = AiV3Normalize(p - output->origin);
				bool outputDone = false;

				// DOF
				if(data->useDof && data->apertureSize >0.0f){
					float lensU;
					float lensV;
					ConcentricSampleDisk(input->lensx, input->lensy, data->apertureBlades, data->apertureBladeCurvature, data->apertureRotation, &lensU, &lensV, data->bokehInvert, data->bokehBias, data->bokehGain);
					lensU*=data->apertureSize;
					lensV*=data->apertureSize;
					float ft = data->focusPlaneIsPlane?std::abs(data->focalDistanceR/dir.z):data->focalDistanceR;
					AtPoint Pfocus = dir*ft;

					// Focal Aspect Ratio
					lensV*=data->apertureAspectRatio;

					output->origin.x = lensU;
					output->origin.y = lensV;
					output->origin.z = 0.0;
					dir = AiV3Normalize(Pfocus-output->origin);
				}

				if(!outputDone)
				{
					// Rotate
					AtVector dir2;
					AiM4VectorByMatrixMult(&dir2,data->center2rightCameraMatrix,&dir);
					output->dir = AiV3Normalize(dir2);

					// Move origin
					const AtPoint po = {output->origin.x,output->origin.y,output->origin.z};
					AiM4PointByMatrixMult(&output->origin,data->center2rightCameraMatrix,&po);
				}
			}
			break;
		}
	case STEREODU:
		{
			// Equal derivatives y is ok, x is bad
			dsy=dsx;

			// Left Camera
			if(input->sy < 0)
			{
				AtPoint p = { input->sx+data->leftCenterOffset , (input->sy+0.5f/(aspect/data->overscanRatio))*2.0f*data->overscanRatio, -1.0f/data->tan_myFov };
				AtVector dir = AiV3Normalize(p - output->origin);
				bool outputDone = false;

				// DOF
				if(data->useDof && data->apertureSize >0.0f){
					float lensU;
					float lensV;
					ConcentricSampleDisk(input->lensx, input->lensy, data->apertureBlades, data->apertureBladeCurvature, data->apertureRotation, &lensU, &lensV, data->bokehInvert, data->bokehBias, data->bokehGain);
					lensU*=data->apertureSize;
					lensV*=data->apertureSize;
					float ft = data->focusPlaneIsPlane?std::abs(data->focalDistanceL/dir.z):data->focalDistanceL;
					AtPoint Pfocus = dir*ft;

					// Focal Aspect Ratio
					lensV*=data->apertureAspectRatio;

					output->origin.x = lensU;
					output->origin.y = lensV;
					output->origin.z = 0.0;
					dir = AiV3Normalize(Pfocus-output->origin);
				}

				if(!outputDone)
				{
					// Rotate
					AtVector dir2;
					AiM4VectorByMatrixMult(&dir2,data->center2leftCameraMatrix,&dir);
					output->dir = AiV3Normalize(dir2);

					// Move origin
					const AtPoint po = {output->origin.x,output->origin.y,output->origin.z};
					AiM4PointByMatrixMult(&output->origin,data->center2leftCameraMatrix,&po);
				}

			}
			else // Right camera
			{
				AtPoint p = { input->sx+data->rightCenterOffset , (input->sy-0.5f/(aspect/data->overscanRatio))*2.0f*data->overscanRatio, -1.0f/data->tan_myFov };
				AtVector dir = AiV3Normalize(p - output->origin);
				bool outputDone = false;

				// DOF
				if(data->useDof && data->apertureSize >0.0f){
					float lensU;
					float lensV;
					ConcentricSampleDisk(input->lensx, input->lensy, data->apertureBlades, data->apertureBladeCurvature, data->apertureRotation, &lensU, &lensV, data->bokehInvert, data->bokehBias, data->bokehGain);
					lensU*=data->apertureSize;
					lensV*=data->apertureSize;
					float ft = data->focusPlaneIsPlane?std::abs(data->focalDistanceR/dir.z):data->focalDistanceR;
					AtPoint Pfocus = dir*ft;

					// Focal Aspect Ratio
					lensV*=data->apertureAspectRatio;

					output->origin.x = lensU;
					output->origin.y = lensV;
					output->origin.z = 0.0;
					dir = AiV3Normalize(Pfocus-output->origin);
				}

				if(!outputDone)
				{
					// Rotate
					AtVector dir2;
					AiM4VectorByMatrixMult(&dir2,data->center2rightCameraMatrix,&dir);
					output->dir = AiV3Normalize(dir2);

					// Move origin
					const AtPoint po = {output->origin.x,output->origin.y,output->origin.z};
					AiM4PointByMatrixMult(&output->origin,data->center2rightCameraMatrix,&po);
				}

			}
			break;
		}
	default:
		AtPoint p = { input->sx, input->sy, -1.0f/data->tan_myFov };
		output->dir = AiV3Normalize(p - output->origin);
		break;
	}

	///////////////////////////////////
	// Derivatives thanks to Alan King
	///////////////////////////////////
	AtVector d = output->dir*std::abs(((-1.0f/data->tan_myFov)/output->dir.z));

	float d_dot_d = AiV3Dot(d, d);
	float temp = 1.0f / std::sqrt(d_dot_d * d_dot_d * d_dot_d);

	// Already initialized to 0's, only compute the non zero coordinates
	output->dDdx.x = (d_dot_d * dsx - (d.x * dsx) * d.x) * temp;
	output->dDdx.y = (              - (d.x * dsx) * d.y) * temp;
	output->dDdx.z = (              - (d.x * dsx) * d.z) * temp;

	output->dDdy.x = (              - (d.y * dsy) * d.x) * temp;
	output->dDdy.y = (d_dot_d * dsy - (d.y * dsy) * d.y) * temp;
	output->dDdy.z = (              - (d.y * dsy) * d.z) * temp;
}

//node_loader
//{
//	if (i != 0)
//		return FALSE;
//	node->methods     = ObqStereoLensMethods;
//	node->output_type = AI_TYPE_UNDEFINED;
//	node->name        = "Obq_StereoLens";
//	node->node_type   = AI_NODE_CAMERA;
//	strcpy(node->version, AI_VERSION);
//	return TRUE;
//}
