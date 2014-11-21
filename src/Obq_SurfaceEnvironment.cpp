/*
Obq_SurfaceEnvironment.cpp :

	Based on sib_environment and sphericalLightFilter by nozon.

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

#include "Obq_SurfaceEnvironment.h"


node_parameters
{

	AiParameterRGBA("tex",0.0f,0.0f,0.0f,0.0f);
	AiParameterBOOL ( "considerAlpha",false);
	AiParameterINT ( "mapMode", 2 );
	AiParameterBOOL ( "usePassEnvShader",false);
	AiParameterINT ( "rayTypeEnv", 2 );
	AiParameterBOOL ( "useIntensityEnv",false); 
	AiParameterBOOL ( "useTransformsEnv",false); 
	AiParameterBOOL ( "useRoughnessEnv",false);
	AiParameterINT ( "sampleLevel", 1 );
	AiParameterFLT ( "coneAngle", 10 );
	AiParameterFLT ( "cosLobeGloss", 0.0f );
	AiParameterBOOL ( "useSampleCount",false);
	AiParameterFLT ( "sampleCount",10.0f);
	AiParameterFLT ( "sampleCountMultiplier",1.0f);
	AiParameterINT ( "onSurfaceMode", 0 );
	AiParameterFLT ( "ior"   , 1.0f );
	AiParameterVEC ( "customDirection"   , 0.0f, 1.0f, 0.0f );
	AiParameterFLT ( "intensity"   , 1.0f );
	AiParameterBOOL ( "globalRotation", true);
	AiParameterVEC ( "rotation"   , 0.0f, 0.0f, 0.0f );
	AiParameterBOOL ( "passthrough", false);
	AiParameterRGBA("passthroughShader",0.0f,0.0f,0.0f,1.0f);	// Color passthrough
	AiParameterSTR("renderChannel","");
	AiParameterBOOL ( "flipU", false);
	AiParameterBOOL ( "flipV", false);

}



node_initialize
{

	ShaderData *data = (ShaderData*) AiMalloc(sizeof(ShaderData));
	AiNodeSetLocalData(node,data);

	data->sampler = NULL;

}



node_update
{

	// Access shader Data
	ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);

	// Get Rotation vector
	AtVector rotation = AiNodeGetVec(node,"rotation");

	// Rotation Matrix
	AtMatrix mx, my, mz, m;

	AiM4Identity(m);
	AiM4RotationX(mx,-rotation.x);
	AiM4RotationY(my,rotation.y);
	AiM4RotationZ(mz,-rotation.z);

	AiM4Mult(m, m, mz);
	AiM4Mult(m, m, my);


	// Mult matrix
	AiM4Mult(data->rotMatrix, m, mx);

	// MapModes
	data->mapMode = AiNodeGetInt(node,"mapMode");

	// Destroy the sampler
	AiSamplerDestroy(data->sampler);

	// Get sampleLevel
	int sampleLevel = AiNodeGetInt(node, "sampleLevel");

	// Initializing AiSampler for multi-ray
	if(sampleLevel > 1)
		data->sampler = AiSampler(sampleLevel, 2);
	else
		data->sampler = NULL;

}



node_finish
{

	ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);

	AiSamplerDestroy(data->sampler);

	// Desallocate shader data
	AiFree(data);
}



shader_evaluate
{

	// Save uv
	float orig_u = sg->u;
	float orig_v = sg->v;

	// flip
	bool flipU = AiShaderEvalParamBool(p_flipU);
	bool flipV = AiShaderEvalParamBool(p_flipV);

	// Intensity
	float intensity = AiShaderEvalParamFlt(p_intensity);

	bool usePassEnvShader = AiShaderEvalParamBool(p_usePassEnvShader);
	bool useIntensityEnv = AiShaderEvalParamBool(p_useIntensityEnv);
	bool useTransformsEnv = AiShaderEvalParamBool(p_useTransformsEnv);
	bool useRoughnessEnv = AiShaderEvalParamBool(p_useRoughnessEnv);

	/////////////////////
	// Texture Evaluation
	/////////////////////

	// Trivial case
	if(intensity == 0.0f && !(usePassEnvShader && !useIntensityEnv))
	{
		sg->out.RGBA = AI_RGBA_BLACK;
	}
	else
	{

		// Access shader Data
		ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);


		// Direction
		AtVector dir;
		AtVector dir2;
		int onsmode = AiShaderEvalParamInt(p_onSurfaceMode);
		

		int sampleLevel = AiShaderEvalParamInt(p_sampleLevel);
		float coneAngle = AiShaderEvalParamFlt(p_coneAngle)/2.0f;
		float gloss = AiShaderEvalParamFlt(p_cosLobeGloss);

		// Ray Type
		unsigned int rayType;

		if( usePassEnvShader ){

			if(!useIntensityEnv)
				intensity=1.0f;

			switch(AiShaderEvalParamInt(p_rayTypeEnv)){
			case 0:
				rayType = AI_RAY_CAMERA;
				break;
			case 1:
				rayType = AI_RAY_DIFFUSE;
				break;
			case 2:
				rayType = AI_RAY_REFLECTED;
				break;
			case 3:
				rayType = AI_RAY_GLOSSY;
				break;
			case 4:
				rayType = AI_RAY_REFRACTED;
				break;
			default:
				rayType = AI_RAY_CAMERA;
				break;
			}
		}

		switch(onsmode){
		case D_VIEW:
			dir2 = sg->Rd;
			break;
		case D_VIEW_INV:
			dir2 = -sg->Rd;
			break;
		case D_NOR:
			dir2 = sg->N;
			break;
		case D_NOR_SMOOTH:
			dir2 = sg->Ns;
			break;
		case D_NOR_INV:
			dir2 = -sg->N;
			break;
		case D_NOR_INV_SMOOTH:
			dir2 = -sg->Ns;
			break;
		case D_NOR_FF:
			dir2 = sg->Nf;
			break;
		case D_NOR_FF_INV:
			dir2 = -sg->Nf;
			break;
		case D_RFL:
			AiReflectSafe(&sg->Rd,&sg->Nf,&sg->Ng,&dir2);
			break;
		case D_RFL_SMOOTH:
			AiReflectSafe(&sg->Rd,&sg->Ns,&sg->Ng,&dir2);
			break;
		case D_RFR:
			AiRefract(&sg->Rd,&sg->Nf,&dir2,1.0f,AiShaderEvalParamFlt(p_ior));
			break;
		case D_RFR_SMOOTH:
			AiRefract(&sg->Rd,&sg->Ns,&dir2,1.0f,AiShaderEvalParamFlt(p_ior));
			break;
		case D_CUSTOM:
			dir2 = AiShaderEvalParamVec(p_customDirection);
			dir2 = AiV3Normalize(dir2);
			break;
		}

		// Rotation only if not in pass shader mode
		if(usePassEnvShader && !useTransformsEnv )
		{
			dir = dir2;
		}
		else if( AiShaderEvalParamBool(p_globalRotation))	//global rotation
		{
			// Rotate vector 
			AiM4VectorByMatrixMult(&dir, data->rotMatrix, &dir2);
		}
		else	// rotation info
		{
			// Get Rotation vector
			AtVector rotation = AiShaderEvalParamVec(p_rotation);

			// Rotation Matrix
			AtMatrix mx, my, mz, m;

			AiM4Identity(m);
			AiM4RotationX(mx,-rotation.x);
			AiM4RotationY(my,rotation.y);
			AiM4RotationZ(mz,-rotation.z);

			AiM4Mult(m, m, mz);
			AiM4Mult(m, m, my);
			AiM4Mult(m, m, mx);

			AiM4VectorByMatrixMult  (&dir, m, &dir2);

		}



		// Simple_case
		if((sampleLevel == 1 || coneAngle == 0.0f || gloss == 1.0f ) || (usePassEnvShader && !useRoughnessEnv) )
		{

			if(usePassEnvShader)
			{
				AtScrSample  scrSample;
				AtRay newRay;
				AiMakeRay(&newRay, rayType, &sg->P, &dir, AI_BIG, sg);
				AiTraceBackground (&newRay, &scrSample);

				// Get color
				sg->out.RGBA.r = scrSample.color.r;
				sg->out.RGBA.g = scrSample.color.g;
				sg->out.RGBA.b = scrSample.color.b;
				sg->out.RGBA.a = scrSample.alpha;

				// Scale with intensity
				ObqRGBAScale(sg->out.RGBA, intensity);

			}
			else
			{
				// Mapping
				switch(data->mapMode)
				{
				case MIRROREDBALL :
					AiMappingMirroredBall(&dir, &sg->u, &sg->v); 
					break;
				case ANGULARMAP :
					AiMappingAngularMap(&dir, &sg->u, &sg->v);
					break;
				case LATLONG :
					AiMappingLatLong(&dir, &sg->u, &sg->v); 
					break;
				case CUBICMAP :
					AiMappingCubicMap(&dir, &sg->u, &sg->v); 
					break;
				}

				//flip
				if(flipU)
					sg->u = 1.0f-sg->u;
				if(flipV)
					sg->v = 1.0f-sg->v;

				// Get color
				sg->out.RGBA = AiShaderEvalParamRGBA(p_tex);

				// Scale with intensity
				ObqRGBAScale(sg->out.RGBA, intensity);
			}
		}
		else
		{
			// Use sample count
			bool useSampleCount =  AiShaderEvalParamBool(p_useSampleCount);
			int sampleCount = int(std::floor(0.5f + AiShaderEvalParamFlt(p_sampleCount)*AiShaderEvalParamFlt(p_sampleCountMultiplier)));

			// Sample count make 0
			if(useSampleCount && sampleCount <= 1 ){

				if(usePassEnvShader)
				{

					AtScrSample  scrSample;
					AtRay newRay;
					AiMakeRay(&newRay, rayType, &sg->P, &dir, AI_BIG, sg);
					AiTraceBackground (&newRay, &scrSample);

					// Get color
					sg->out.RGBA.r = scrSample.color.r;
					sg->out.RGBA.g = scrSample.color.g;
					sg->out.RGBA.b = scrSample.color.b;
					sg->out.RGBA.a = scrSample.alpha;

					// Scale with intensity
					ObqRGBAScale(sg->out.RGBA, intensity);
				}
				else
				{
					// Mapping
					switch(data->mapMode)
					{
					case MIRROREDBALL :
						AiMappingMirroredBall(&dir, &sg->u, &sg->v); 
						break;
					case ANGULARMAP :
						AiMappingAngularMap(&dir, &sg->u, &sg->v);
						break;
					case LATLONG :
						AiMappingLatLong(&dir, &sg->u, &sg->v); 
						break;
					case CUBICMAP :
						AiMappingCubicMap(&dir, &sg->u, &sg->v); 
						break;
					}

					//flip
					if(flipU)
						sg->u = 1.0f-sg->u;
					if(flipV)
						sg->v = 1.0f-sg->v;

					// Get color
					sg->out.RGBA = AiShaderEvalParamRGBA(p_tex);

					// Scale with intensity
					ObqRGBAScale(sg->out.RGBA, intensity);
				}

			}
			else		
			{
				////////////////
				// Need samples
				////////////////

				// For Sampler & environment checking
				sg->out.RGBA = AI_RGBA_BLACK;

#if OBQ_AI_VERSION >= 40100
				float rndTmp[2];
#else
				double rndTmp[2];
#endif
				float rnd[2];
				AtSamplerIterator* iter;
				
				// Direction
				AtVector V = dir;

				// Exponent
				float cosLobeExponent = GlossToExponent(gloss);

				float maxAngle = float(coneAngle*AI_DTOR);
				AtVector Vperp = {(V.z - V.y), (V.x - V.z), (V.y - V.x)}; // A perpendicular vector
				Vperp = AiV3Normalize(Vperp);
				AtVector KcrossV = AiV3Cross(V, Vperp);

				// Get an iterator to loop over the samples
				iter = AiSamplerIterator(data->sampler, sg);
				AiSamplerGetSampleCount(iter);

				if(sampleCount < 1)
					sampleCount = 1;
				else if(sampleCount > sampleLevel*sampleLevel)
					sampleCount = sampleLevel*sampleLevel;

				int iSample = 0, nSamples = useSampleCount?sampleCount:sampleLevel*sampleLevel;

				while(iSample < nSamples && AiSamplerGetSample(iter, rndTmp)) 
				{ 
					rnd[0] = static_cast<float>(rndTmp[0]);
					rnd[1] = static_cast<float>(rndTmp[1]);

					// Get theta phi
					float theta, phi;
					UVToSpherical(static_cast<float>(rnd[0]),static_cast<float>(rnd[1]),maxAngle,cosLobeExponent,&theta, &phi);

					// Simplified rodrigues rotations for theta
					AtVector Vt = V*std::cos(theta) + KcrossV*std::sin(theta);

					// Phi
					AtVector R = RodriguesRotation(Vt,V,phi);

					// Value accumulation
					if(usePassEnvShader)
					{
						AtScrSample  scrSample;
						AtRay newRay;
						AiMakeRay(&newRay, rayType, &sg->P, &R, AI_BIG, sg);
						AiTraceBackground (&newRay, &scrSample);

						// Get color
						sg->out.RGBA.r += scrSample.color.r;
						sg->out.RGBA.g += scrSample.color.g;
						sg->out.RGBA.b += scrSample.color.b;
						sg->out.RGBA.a += scrSample.alpha;


					}
					else
					{

						// Mapping
						switch(data->mapMode)
						{
						case MIRROREDBALL :
							AiMappingMirroredBall(&R, &sg->u, &sg->v); 
							break;
						case ANGULARMAP :
							AiMappingAngularMap(&R, &sg->u, &sg->v);
							break;
						case LATLONG :
							AiMappingLatLong(&R, &sg->u, &sg->v); 
							break;
						case CUBICMAP :
							AiMappingCubicMap(&R, &sg->u, &sg->v); 
							break;
						}

						//flip
						if(flipU)
							sg->u = 1.0f-sg->u;
						if(flipV)
							sg->v = 1.0f-sg->v;

						ObqRGBAAdd(sg->out.RGBA,AiShaderEvalParamRGBA(p_tex));
					}

					iSample++;
				}

				// Multiply by inv sample count
				intensity/=float(iSample);

				// Scale with intensity
				ObqRGBAScale(sg->out.RGBA,intensity);

			}
		}
	}

	// Restore uv
	sg->u = orig_u;
	sg->v = orig_v;

	// Check for currupted value
	if(AiColorCorrupted(sg->out.RGBA)){
		AiMsgWarning("!! Obq_SurfaceEnvironment : Corrupted color found !!");
		sg->out.RGBA = AI_RGBA_BLACK;
	}

	// Alpha of texture
	if(!AiShaderEvalParamBool(p_considerAlpha))
		sg->out.RGBA.a = 1.0;

	// renderChannel
	const char* renderChannel = AiShaderEvalParamStr(p_renderChannel);
	if(AiAOVEnabled(renderChannel, AI_TYPE_RGBA))
		AiAOVSetRGBA (sg, renderChannel , sg->out.RGBA);

	// passthrough ?
	if(AiShaderEvalParamBool(p_passthrough))
		sg->out.RGBA = AiShaderEvalParamRGBA(p_passthroughShader);


}

//node_loader
//{
//	if (i > 0)
//		return FALSE;
//
//	node->methods      = ObqSurfaceEnvironmentMethods;
//	node->output_type  = AI_TYPE_RGBA;
//	node->name         = "Obq_SurfaceEnvironment";
//	node->node_type    = AI_NODE_SHADER;
//	strcpy(node->version, AI_VERSION);
//	return TRUE;
//}
