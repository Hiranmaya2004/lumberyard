/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include <Tests/UI/UIFixture.h>
#include <Tests/UI/AnimGraphUIFixture.h>
#include <Integration/System/SystemCommon.h>

#include <EMotionFX/Source/AnimGraphManager.h>
#include <EMotionFX/Source/AnimGraphObject.h>

#include <EMotionStudio/EMStudioSDK/Source/EMStudioManager.h>
#include <EMotionStudio/EMStudioSDK/Source/PluginManager.h>
#include <EMotionStudio/Plugins/StandardPlugins/Source/AnimGraph/BlendGraphViewWidget.h>
#include <EMotionStudio/Plugins/StandardPlugins/Source/AnimGraph/BlendGraphWidget.h>
#include <EMotionStudio/Plugins/StandardPlugins/Source/AnimGraph/AnimGraphPlugin.h>

#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzQtComponents/Components/StyleManager.h>
#include <AzQtComponents/Utilities/QtPluginPaths.h>

#include <EMotionFX/Source/AnimGraphReferenceNode.h>
#include <EMotionFX/Source/AnimGraphObjectFactory.h>
#include <EMotionStudio/EMStudioSDK/Source/EMStudioManager.h>

#include <QApplication>
#include <QWidget>

namespace EMotionFX
{
    void AnimGraphUIFixture::SetUp()
    {
        UIFixture::SetUp();

        m_animGraphPlugin = static_cast<EMStudio::AnimGraphPlugin*>(EMStudio::GetPluginManager()->FindActivePlugin(EMStudio::AnimGraphPlugin::CLASS_ID));
        ASSERT_TRUE(m_animGraphPlugin) << "Anim graph plugin not found.";
        EXPECT_FALSE(m_animGraphPlugin->GetActiveAnimGraph()) << "No anim graph should be activated.";
        EXPECT_EQ(0, EMotionFX::GetAnimGraphManager().GetNumAnimGraphs()) << "Anim graph manager should contain 0 anim graph.";

        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        m_blendGraphWidget = m_animGraphPlugin->GetGraphWidget();
        ASSERT_TRUE(m_blendGraphWidget) << "BlendGraphWidget not found";
    }

    EMotionFX::AnimGraph* AnimGraphUIFixture::CreateAnimGraph() const
    {
        auto addAnimGraphAction = m_animGraphPlugin->GetViewWidget()->findChild<QAction*>("EMFX.BlendGraphViewWidget.NewButton");
        if (!addAnimGraphAction)
        {
            return nullptr;
        }

        addAnimGraphAction->trigger();

        auto animGraph = m_animGraphPlugin->GetActiveAnimGraph();
        // The empty graph should contain one node (the root statemachine).
        EXPECT_TRUE(animGraph && animGraph->GetNumNodes() == 1) << "An empty anim graph should be activated.";
        EXPECT_EQ(1, EMotionFX::GetAnimGraphManager().GetNumAnimGraphs()) << "Anim graph manager should contain 1 anim graph.";

        return animGraph;
    }

    EMStudio::NodeGraph* AnimGraphUIFixture::GetActiveNodeGraph() const
    {
        return m_blendGraphWidget->GetActiveGraph();
    }

    EMotionFX::AnimGraphNode* AnimGraphUIFixture::CreateAnimGraphNode(const AZStd::string &type, const AZStd::string &args, const AnimGraph* animGraph) const
    {
        // Creates an AnimGraphNode by executing command through the CommandManager.
        // Parameters:
        //     AZStd::string type - the id string of the node type to create
        //     AZStd::string args [optional] - optional string arguments to pass to the command.
        //     AnimGraph* animGraph [optional] - The AnimGraph to add the new Node to. Defaults to the current Active AnimGraph.
        
        const AnimGraph* targetAnimGraph = (animGraph ? animGraph : m_animGraphPlugin->GetActiveAnimGraph()); //AnimGraph to add Node to
        const AZStd::string cmd = "AnimGraphCreateNode AnimGraphID " + AZStd::to_string(targetAnimGraph->GetID()) + " -type " + type + " " + args;
        
        AZ::u32 nodeCount = targetAnimGraph->GetNumNodes(); //node count before creating a new node

        AZStd::string result;
        EXPECT_TRUE(CommandSystem::GetCommandManager()->ExecuteCommand(cmd, result)) << result.c_str();
        EXPECT_EQ(targetAnimGraph->GetNumNodes(), nodeCount + 1) << "Expected one more anim graph node after running command: " << cmd.c_str();

        return targetAnimGraph->GetNode(targetAnimGraph->GetNumNodes() - 1);
    }
} // namespace EMotionFX