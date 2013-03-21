/*
Obq_RandomColor v2.06.0a (win64 - SItoA 2.6.0a - Arnold 4.0.11.0):

is a shader that tells information concerning the thickness of a surface in a certain direction
or the number of objects in this direction

*------------------------------------------------------------------------
Copyright (c) 2012 Marc-Antoine Desjardins, ObliqueFX (madesjardins@obliquefx.com)

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

#include "ai.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <list>
#include <string>

// Arnold Thingy
//
AI_SHADER_NODE_EXPORT_METHODS(ObqRandomColorSimpleMethods);

// enum of parameters
//
enum ObqRandomColorSimpleParams {p_randMax, p_seed, p_color01, p_color02, p_color03, p_color04, p_color05, p_color06, p_color07, p_color08, p_color09, p_color10, p_color11, p_color12, p_color13, p_color14, p_color15, p_color16, p_stripModelName, p_stripFrameNumber};



// parameters
//
node_parameters
{
	AiParameterINT("randMax", 4);	
	AiParameterINT("seed", 233);
	AiParameterRGBA("color01", 0.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color02", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color03", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color04", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color05", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color06", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color07", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color08", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color09", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color10", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color11", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color12", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color13", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color14", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color15", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterRGBA("color16", 1.0f, 0.0f,0.0f,0.0f);
	AiParameterBOOL("stripModelName", true);
	AiParameterBOOL("stripFrameNumber", true);
}

node_initialize
{

}

node_update
{

}


node_finish
{

}

shader_evaluate
{

	std::string name(AiNodeGetName(sg->Op));

	// stripping
	std::size_t len = name.length();
	std::size_t endNameIndex = name.rfind(".SItoA.");
	std::size_t startNameIndex = 0;
	
	if(endNameIndex==std::string::npos)
	{
		AiMsgError("[Obq_RandomColor] : Couldn't find the frame number separator. Name is : %s and separator is .SItoA.",name.c_str());
		sg->out.RGBA = AI_RGBA_BLACK;
		return; 
	}

	// Strip model name if any
	if(AiShaderEvalParamBool(p_stripModelName))
	{
		startNameIndex = name.find('.');
		if(startNameIndex >= endNameIndex)
			startNameIndex = 0;
		else
			++startNameIndex;
	}

	// Don't strip the frame #
	if(!AiShaderEvalParamBool(p_stripFrameNumber) || endNameIndex >= len)
		endNameIndex = len;

	// Add character
	unsigned int a = AiShaderEvalParamInt(p_seed);

	for(std::size_t i = startNameIndex; i < endNameIndex; i++)
		//a += unsigned int(name[i]*name[i]*name[i])%31;
		a += (unsigned int)(name[i]*name[i])%73;
		//a += unsigned int(name[i]);

	// thanks to Mike F : http://stackoverflow.com/questions/167735/fast-pseudo-random-number-generator-for-procedural-content
    a = (a ^ 61) ^ (a >> 16);
    a = a + (a << 3);
    a = a ^ (a >> 4);
    a = a * 0x27d4eb2d;
    a = a ^ (a >> 15);
    unsigned int value = (a%AiShaderEvalParamInt(p_randMax))+1;

	AtRGBA color;
	switch(value)
	{
	case 1 : 
		color = AiShaderEvalParamRGBA(p_color01);
		break;
	case 2 : 
		color = AiShaderEvalParamRGBA(p_color02);
		break;
	case 3 : 
		color = AiShaderEvalParamRGBA(p_color03);
		break;
	case 4 : 
		color = AiShaderEvalParamRGBA(p_color04);
		break;
	case 5 : 
		color = AiShaderEvalParamRGBA(p_color05);
		break;
	case 6 : 
		color = AiShaderEvalParamRGBA(p_color06);
		break;
	case 7 : 
		color = AiShaderEvalParamRGBA(p_color07);
		break;
	case 8 : 
		color = AiShaderEvalParamRGBA(p_color08);
		break;
	case 9 : 
		color = AiShaderEvalParamRGBA(p_color09);
		break;
	case 10 : 
		color = AiShaderEvalParamRGBA(p_color10);
		break;
	case 11 : 
		color = AiShaderEvalParamRGBA(p_color11);
		break;
	case 12 : 
		color = AiShaderEvalParamRGBA(p_color12);
		break;
	case 13 : 
		color = AiShaderEvalParamRGBA(p_color13);
		break;
	case 14 : 
		color = AiShaderEvalParamRGBA(p_color14);
		break;
	case 15 : 
		color = AiShaderEvalParamRGBA(p_color15);
		break;
	case 16 : 
		color = AiShaderEvalParamRGBA(p_color16);
		break;
	default:
		color = AiShaderEvalParamRGBA(p_color01);
		break;
	}

	sg->out.RGBA = color;

}

//node_loader
//{
//	if (i > 0)
//		return FALSE;
//
//	node->methods      = ObqRandomColorSimpleMethods;
//	node->output_type  = AI_TYPE_RGBA;
//	node->name         = "Obq_RandomColor";
//	node->node_type    = AI_NODE_SHADER;
//	strcpy(node->version, AI_VERSION);
//	return TRUE;
//}