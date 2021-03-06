# 2016-12-18 5.46 AM

import pymel.core as pm
import maya.cmds as cmds
import maya.mel as mel
import mtoa.utils as utils
import mtoa.ui.ae.utils as aeUtils
from mtoa.ui.ae.shaderTemplate import ShaderAETemplate

def Obq_LightSaturationFilterHelpURL():
	# Add the Obq_Shader docs URL to the Attribute Editor help menu
	ObqNodeType = 'Obq_LightSaturationFilter'
	ObqNodeHelpURL = 'https://github.com/madesjardins/Obq_Shaders/wiki/Obq_LightSaturationFilter'
	ObqHelpCommand = 'addAttributeEditorNodeHelp("' + ObqNodeType + '", "showHelp -absolute \\"' +ObqNodeHelpURL +'\\"");'
	mel.eval(ObqHelpCommand)

class AEObq_LightSaturationFilterTemplate(ShaderAETemplate):
	convertToMayaStyle = True

	def setup(self):

		# Hide the node swatch preview until the shader is able to render a preview
		#self.addSwatch()

		self.beginScrollLayout()

		self.addCustom('message', 'AEshaderTypeNew', 'AEshaderTypeReplace')

		#pm.picture(image="Obq_shader_header.png", parent="AttrEdObq_LightSaturationFilterFormLayout")
		Obq_LightSaturationFilterHelpURL()

		self.beginLayout("Main", collapse=False)
		self.addControl("mode", label="Mode")
		self.addControl("key", label="Key")
		self.addControl("triggerValue", label="Trigger Value")
		self.addControl("saturation", label="Saturation")
		self.addControl("defaultSaturation", label="Default Saturation")
		self.endLayout()

		# include/call base class/node attributes
		pm.mel.AEdependNodeTemplate(self.nodeName)

		# Hide the NormalCamera and HardwareColor Extra Attributes
		self.suppress('normalCamera')
		self.suppress('hardwareColor')
	  
		self.addExtraControls()
		self.endScrollLayout()
