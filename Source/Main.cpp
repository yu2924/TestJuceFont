//
//  Main.cpp
//  TestJuceFont_App
//
//  created by yu2924 on 2023-05-20
//

#include <JuceHeader.h>
#include "PlatformGlue.h"

void setupEnvironmentFont()
{
	juce::String typefacename = PlatformGlue::getSystemSansSerifTypefaceName();
	DBG("DefaultSansSerifTypefaceName=" << typefacename.quoted());
#define SELECT_TYPEFACE_BYNAME 0
#if SELECT_TYPEFACE_BYNAME
	juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName(typefacename);
#else
	juce::Font font;
	font.setTypefaceName(typefacename);
	juce::Typeface::Ptr tf = font.getTypefacePtr();
	juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(tf);
#endif
}

juce::Font getFontWithTypefaceNameAndPointHeightAndStyle(const juce::String typefacename, const juce::String& typefacestyle, float pointheight)
{
	juce::Font tmpfont;
	tmpfont.setTypefaceName(typefacename);
	tmpfont.setTypefaceStyle(typefacestyle);
	juce::Typeface::Ptr tf = tmpfont.getTypefacePtr();
	float scale = tf->getHeightToPointsFactor();
	float fontheight = pointheight / scale;
	return juce::Font(typefacename, typefacestyle, fontheight);
}

class MainComponent : public juce::Component
{
private:
	struct Renderer
	{
		juce::ComponentPeer* peer = nullptr;
		juce::StringArray engineNameList;
		int engineIndex = 0;
		std::function<void()> onEngineListDidUpdate;
		std::function<void()> onEngineDidChange;
		Renderer()
		{
		}
		void updatePeer(juce::Component* cmp)
		{
			juce::ComponentPeer* p = cmp->getPeer();
			if(peer == p) return;
			peer = p;
			engineNameList = peer ? peer->getAvailableRenderingEngines() : juce::StringArray();
			if(onEngineListDidUpdate) onEngineListDidUpdate();
			setEngineIndex(engineIndex);
		}
		void setEngineIndex(int ieng)
		{
			engineIndex = !engineNameList.isEmpty() ? std::max(0, std::min(engineNameList.size() - 1, ieng)) : 0;
			if(peer)
			{
				peer->setCurrentRenderingEngine(engineIndex);
				DBG("renderingengine=" << engineNameList[engineIndex].quoted());
			}
			if(onEngineDidChange) onEngineDidChange();
		}
	} renderer;
	juce::ComboBox engineCombo;
	juce::ComboBox typefaceCombo;
	juce::ComboBox styleCombo;
	juce::ComboBox fontSizeCombo;
	juce::Label infoLabel;
	juce::TextEditor textEditor;
	juce::Label displayLabel;
	enum { Margin = 8, CBHeight = 24, EngineWidth = 160, TypefaceWidth = 160, StyleWidth = 80, FontsizeWidth = 80, InfoWidth = 160 };
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
public:
	MainComponent()
	{
		renderer.onEngineListDidUpdate = [this]()
		{
			engineCombo.clear(juce::dontSendNotification);
			engineCombo.addItemList(renderer.engineNameList, 1);
			engineCombo.setSelectedItemIndex(renderer.engineIndex, juce::dontSendNotification);
		};
		renderer.onEngineDidChange = [this]()
		{
			engineCombo.setSelectedItemIndex(renderer.engineIndex, juce::dontSendNotification);
		};
		addAndMakeVisible(engineCombo);
		engineCombo.onChange = [this]()
		{
			renderer.setEngineIndex(engineCombo.getSelectedItemIndex());
		};
		addAndMakeVisible(typefaceCombo);
		typefaceCombo.onChange = [this]()
		{
			fillStyle();
			updateFont();
		};
		addAndMakeVisible(styleCombo);
		styleCombo.onChange = [this]()
		{
			updateFont();
		};
		addAndMakeVisible(fontSizeCombo);
		fontSizeCombo.setEditableText(true);
		fontSizeCombo.addItemList(juce::StringArray({ 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72 }), 1);
		fontSizeCombo.setText(juce::String(juce::Font().getHeight()), juce::dontSendNotification);
		fontSizeCombo.onChange = [this]()
		{
			updateFont();
		};
		addAndMakeVisible(infoLabel);
		infoLabel.setFont(juce::Font().withPointHeight(12));
		infoLabel.setJustificationType(juce::Justification::topLeft);
		addAndMakeVisible(textEditor);
		textEditor.setMultiLine(true, true);
		textEditor.setReturnKeyStartsNewLine(true);
		textEditor.setTabKeyUsedAsCharacter(true);
		textEditor.onTextChange = [this]()
		{
			displayLabel.setText(textEditor.getText(), juce::dontSendNotification);
		};
		addAndMakeVisible(displayLabel);
		displayLabel.setColour(juce::Label::ColourIds::outlineColourId, getLookAndFeel().findColour(juce::TextEditor::ColourIds::outlineColourId));
		displayLabel.setJustificationType(juce::Justification::topLeft);
		// init
		juce::String deftypeface = juce::Font().getTypefacePtr()->getName();
		juce::StringArray typefacelist = juce::Font::findAllTypefaceNames();
		typefaceCombo.addItemList(typefacelist, 1);
		// first, find exact match
		for(int c = typefacelist.size(), i = 0; i < c; ++i)
		{
			if(typefacelist[i] == deftypeface) { typefaceCombo.setSelectedItemIndex(i, juce::NotificationType::dontSendNotification); break; }
		}
		// next, find partial match
		if(typefaceCombo.getSelectedItemIndex() < 0)
		{
			for(int c = typefacelist.size(), i = 0; i < c; ++i)
			{
				if(typefacelist[i].startsWith(deftypeface) || deftypeface.startsWith(typefacelist[i])) { typefaceCombo.setSelectedItemIndex(i, juce::NotificationType::dontSendNotification); break; }
			}
		}
		// finally, select the first
		if(typefaceCombo.getSelectedItemIndex() < 0) typefaceCombo.setSelectedItemIndex(0, juce::dontSendNotification);
		fillStyle();
		updateFont();
		setSize(480, 320);
#if JUCE_WINDOWS
		juce::File sampletextfile = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getChildFile("../../../../../sampletext.txt");
#elif JUCE_MAC
		juce::File sampletextfile = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getChildFile("../../../../sampletext.txt");
#endif
		loadTextFile(sampletextfile);
	}
	virtual ~MainComponent() override
	{
	}
	void fillStyle()
	{
		styleCombo.clear();
		juce::String typeface = typefaceCombo.getText();
		juce::StringArray stylelist = juce::Font::findAllTypefaceStyles(typeface);
		styleCombo.addItemList(stylelist, 1);
		styleCombo.setSelectedItemIndex(0, juce::NotificationType::dontSendNotification);
	}
	void updateFont()
	{
		juce::Font font = getFontWithTypefaceNameAndPointHeightAndStyle(typefaceCombo.getText(), styleCombo.getText(), fontSizeCombo.getText().getFloatValue());
		juce::Typeface::Ptr tf = font.getTypefacePtr();
		juce::String info;
		info += "default san serif:\n" + juce::Font().getTypefacePtr()->getName().quoted();
		info += "\n\n";
		info += "selected:\n";
		info += juce::String::formatted("height=%g\n", font.getHeight());
		info += juce::String::formatted("ascent=%g\n", font.getAscent());
		info += juce::String::formatted("descent=%g\n", font.getDescent());
		info += juce::String::formatted("ascent+descent=%g\n", font.getAscent() + font.getDescent());
		info += juce::String::formatted("pointheight=%g\n", font.getHeightInPoints());
		info += juce::String::formatted("leading=%g\n", font.getHeight() - font.getHeightInPoints());
		infoLabel.setText(info, juce::dontSendNotification);
		textEditor.applyFontToAllText(font);
		displayLabel.setFont(font);
	}
	virtual void parentHierarchyChanged() override
	{
		renderer.updatePeer(this);
	}
	virtual void resized() override
	{
		juce::Rectangle<int> rc = getLocalBounds().reduced(Margin);
		juce::Rectangle<int> rccb = rc.removeFromTop(CBHeight);
		engineCombo.setBounds(rccb.removeFromLeft(EngineWidth));
		typefaceCombo.setBounds(rccb.removeFromLeft(TypefaceWidth));
		styleCombo.setBounds(rccb.removeFromLeft(StyleWidth));
		fontSizeCombo.setBounds(rccb.removeFromLeft(FontsizeWidth));
		infoLabel.setBounds(rc.removeFromLeft(InfoWidth));
		textEditor.setBounds(rc.removeFromTop(rc.getHeight() / 2));
		displayLabel.setBounds(rc);
	}
	virtual void paint(juce::Graphics& g) override
	{
		g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	}
	void loadTextFile(const juce::File& path)
	{
		if(!path.existsAsFile() || (65536 <= path.getSize())) return;
		textEditor.setText(path.loadFileAsString());
	}
};

class MainWindow : public juce::DocumentWindow, public juce::FileDragAndDropTarget
{
private:
	std::unique_ptr<MainComponent> mainComponent;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
public:
	MainWindow(juce::String name) : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons)
	{
		setUsingNativeTitleBar(true);
		mainComponent = std::make_unique<MainComponent>();
		setContentNonOwned(mainComponent.get(), true);
#if JUCE_IOS || JUCE_ANDROID
		setFullScreen(true);
#else
		setResizable(true, true);
		centreWithSize(getWidth(), getHeight());
#endif
		setVisible(true);
	}
	virtual void closeButtonPressed() override
	{
		juce::JUCEApplication::getInstance()->systemRequestedQuit();
	}
	virtual bool isInterestedInFileDrag(const juce::StringArray& files) override
	{
		return files.size() == 1;
	}
	virtual void filesDropped(const juce::StringArray& files, int, int) override
	{
		mainComponent->loadTextFile(juce::File(files[0]));
	}
};

class TestJuceFontApplication : public juce::JUCEApplication
{
private:
	std::unique_ptr<MainWindow> mainWindow;
public:
	TestJuceFontApplication() {}
	virtual const juce::String getApplicationName() override { return ProjectInfo::projectName; }
	virtual const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
	virtual bool moreThanOneInstanceAllowed() override { return true; }
	virtual void initialise(const juce::String&) override
	{
		setupEnvironmentFont();
		if(juce::LookAndFeel_V4* lf4 = dynamic_cast<juce::LookAndFeel_V4*>(&juce::LookAndFeel::getDefaultLookAndFeel()))
		{
			lf4->setColourScheme(juce::LookAndFeel_V4::getLightColourScheme());
		}
		mainWindow.reset(new MainWindow(getApplicationName()));
	}
	virtual void shutdown() override
	{
		mainWindow = nullptr;
	}
	virtual void systemRequestedQuit() override
	{
		quit();
	}
	virtual void anotherInstanceStarted(const juce::String&) override
	{
	}
};

START_JUCE_APPLICATION(TestJuceFontApplication)
