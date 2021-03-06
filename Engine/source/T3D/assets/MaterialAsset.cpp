//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef MATERIALASSET_H
#include "MaterialAsset.h"
#endif

#ifndef _ASSET_MANAGER_H_
#include "assets/assetManager.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _TAML_
#include "persistence/taml/taml.h"
#endif

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(MaterialAsset);

ConsoleType(MaterialAssetPtr, TypeMaterialAssetPtr, MaterialAsset, ASSET_ID_FIELD_PREFIX)

//-----------------------------------------------------------------------------

ConsoleGetType(TypeMaterialAssetPtr)
{
   // Fetch asset Id.
   return (*((AssetPtr<MaterialAsset>*)dptr)).getAssetId();
}

//-----------------------------------------------------------------------------

ConsoleSetType(TypeMaterialAssetPtr)
{
   // Was a single argument specified?
   if (argc == 1)
   {
      // Yes, so fetch field value.
      const char* pFieldValue = argv[0];

      // Fetch asset pointer.
      AssetPtr<MaterialAsset>* pAssetPtr = dynamic_cast<AssetPtr<MaterialAsset>*>((AssetPtrBase*)(dptr));

      // Is the asset pointer the correct type?
      if (pAssetPtr == NULL)
      {
         // No, so fail.
         //Con::warnf("(TypeMaterialAssetPtr) - Failed to set asset Id '%d'.", pFieldValue);
         return;
      }

      // Set asset.
      pAssetPtr->setAssetId(pFieldValue);

      return;
   }

   // Warn.
   Con::warnf("(TypeMaterialAssetPtr) - Cannot set multiple args to a single asset.");
}

//-----------------------------------------------------------------------------

MaterialAsset::MaterialAsset()
{
   mShaderGraphFile = "";
   mScriptFile = "";
   mMatDefinitionName = "";
}

//-----------------------------------------------------------------------------

MaterialAsset::~MaterialAsset()
{
   // If the asset manager does not own the asset then we own the
   // asset definition so delete it.
   if (!getOwned())
      delete mpAssetDefinition;
}

//-----------------------------------------------------------------------------

void MaterialAsset::initPersistFields()
{
   // Call parent.
   Parent::initPersistFields();

   //addField("shaderGraph", TypeRealString, Offset(mShaderGraphFile, MaterialAsset), "");
   addField("scriptFile", TypeRealString, Offset(mScriptFile, MaterialAsset), "Path to the file containing the material definition.");
   addField("materialDefinitionName", TypeRealString, Offset(mMatDefinitionName, MaterialAsset), "Name of the material definition this asset is for.");
}

void MaterialAsset::initializeAsset()
{
   // Call parent.
   Parent::initializeAsset();

   compileShader();

   if (Platform::isFile(mScriptFile))
      Con::executeFile(mScriptFile, false, false);
}

void MaterialAsset::onAssetRefresh()
{
   if (Platform::isFile(mScriptFile))
      Con::executeFile(mScriptFile, false, false);

   if (!mMatDefinitionName.isEmpty())
   {
      Material* matDef;
      if (!Sim::findObject(mMatDefinitionName.c_str(), matDef))
      {
         Con::errorf("MaterialAsset: Unable to find the Material %s", mMatDefinitionName.c_str());
         return;
      }

      matDef->reload();
   }
}

//------------------------------------------------------------------------------

void MaterialAsset::compileShader()
{
}

void MaterialAsset::copyTo(SimObject* object)
{
   // Call to parent.
   Parent::copyTo(object);
}

ConsoleMethod(MaterialAsset, compileShader, void, 2, 2, "() - Compiles the material's generated shader, if any. Not yet implemented\n")
{
   object->compileShader();
}

//-----------------------------------------------------------------------------
// GuiInspectorTypeAssetId
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiInspectorTypeMaterialAssetPtr);

ConsoleDocClass(GuiInspectorTypeMaterialAssetPtr,
   "@brief Inspector field type for Material Asset Objects\n\n"
   "Editor use only.\n\n"
   "@internal"
);

void GuiInspectorTypeMaterialAssetPtr::consoleInit()
{
   Parent::consoleInit();

   ConsoleBaseType::getType(TypeMaterialAssetPtr)->setInspectorFieldType("GuiInspectorTypeMaterialAssetPtr");
}

GuiControl* GuiInspectorTypeMaterialAssetPtr::constructEditControl()
{
   // Create base filename edit controls
   GuiControl *retCtrl = Parent::constructEditControl();
   if (retCtrl == NULL)
      return retCtrl;

   // Change filespec
   char szBuffer[512];
   dSprintf(szBuffer, sizeof(szBuffer), "AssetBrowser.showDialog(\"MaterialAsset\", \"AssetBrowser.changeAsset\", %d, %s);",
      mInspector->getComponentGroupTargetId(), mCaption);
   mBrowseButton->setField("Command", szBuffer);

   // Create "Open in ShapeEditor" button
   mSMEdButton = new GuiBitmapButtonCtrl();

   dSprintf(szBuffer, sizeof(szBuffer), "echo(\"Game Object Editor not implemented yet!\");", retCtrl->getId());
   mSMEdButton->setField("Command", szBuffer);

   char bitmapName[512] = "tools/worldEditor/images/toolbar/shape-editor";
   mSMEdButton->setBitmap(bitmapName);

   mSMEdButton->setDataField(StringTable->insert("Profile"), NULL, "GuiButtonProfile");
   mSMEdButton->setDataField(StringTable->insert("tooltipprofile"), NULL, "GuiToolTipProfile");
   mSMEdButton->setDataField(StringTable->insert("hovertime"), NULL, "1000");
   mSMEdButton->setDataField(StringTable->insert("tooltip"), NULL, "Open this file in the Material Editor");

   mSMEdButton->registerObject();
   addObject(mSMEdButton);

   return retCtrl;
}

bool GuiInspectorTypeMaterialAssetPtr::updateRects()
{
   S32 dividerPos, dividerMargin;
   mInspector->getDivider(dividerPos, dividerMargin);
   Point2I fieldExtent = getExtent();
   Point2I fieldPos = getPosition();

   mCaptionRect.set(0, 0, fieldExtent.x - dividerPos - dividerMargin, fieldExtent.y);
   mEditCtrlRect.set(fieldExtent.x - dividerPos + dividerMargin, 1, dividerPos - dividerMargin - 34, fieldExtent.y);

   bool resized = mEdit->resize(mEditCtrlRect.point, mEditCtrlRect.extent);
   if (mBrowseButton != NULL)
   {
      mBrowseRect.set(fieldExtent.x - 32, 2, 14, fieldExtent.y - 4);
      resized |= mBrowseButton->resize(mBrowseRect.point, mBrowseRect.extent);
   }

   if (mSMEdButton != NULL)
   {
      RectI shapeEdRect(fieldExtent.x - 16, 2, 14, fieldExtent.y - 4);
      resized |= mSMEdButton->resize(shapeEdRect.point, shapeEdRect.extent);
   }

   return resized;
}