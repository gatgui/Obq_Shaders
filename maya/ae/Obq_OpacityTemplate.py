# 2016-12-18 5.46 AM

import pymel.core as pm
import maya.cmds as cmds
import maya.mel as mel
import mtoa.utils as utils
import mtoa.ui.ae.utils as aeUtils
from mtoa.ui.ae.shaderTemplate import ShaderAETemplate

   
def Obq_OpacityHelpURL():
	# Add the Obq_Shader docs URL to the Attribute Editor help menu
	ObqNodeType = 'Obq_Opacity'
	ObqNodeHelpURL = 'https://github.com/madesjardins/Obq_Shaders/wiki/Obq_Opacity'
	ObqHelpCommand = 'addAttributeEditorNodeHelp("' + ObqNodeType + '", "showHelp -absolute \\"' +ObqNodeHelpURL +'\\"");'
	mel.eval(ObqHelpCommand)

class AEObq_OpacityTemplate(ShaderAETemplate):
	convertToMayaStyle = True
	
	def setup(self):

		# Hide the node swatch preview until the shader is able to render a preview
		self.addSwatch()

		self.beginScrollLayout()

		self.addCustom('message', 'AEshaderTypeNew', 'AEshaderTypeReplace')

		#pm.picture(image='Obq_shader_header.png', parent="AttrEdObq_OpacityFormLayout")
		Obq_OpacityHelpURL()
		
		self.beginLayout("Color", collapse=False )
		self.addControl("color", label="Color")
		self.endLayout() #End Color
		
		self.beginLayout("Opacity", collapse=False )
		self.addControl("channel", label="Channel")
		self.addControl("opacityRGBA", label="Opacity RGBA")
		self.addControl("opacityScalar", label="Opacity Scalar")
		self.addControl("hard", label="Use hard opacity")
		self.addSeparator()
		self.addControl("threshold", label="Threshold")
		self.addControl("invert", label="Invert")
		self.endLayout() #End Opacity
		
		# include/call base class/node attributes
		pm.mel.AEdependNodeTemplate(self.nodeName)

		# Hide the NormalCamera and HardwareColor Extra Attributes
		self.suppress('normalCamera')
		self.suppress('hardwareColor')

		self.addExtraControls()
		self.endScrollLayout()
