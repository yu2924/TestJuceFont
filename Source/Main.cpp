//
//  Main.cpp
//  TestJuceFont_App
//
//  created by yu2924 on 2023-05-20
//

#include <JuceHeader.h>

#if JUCE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
void setupEnvironmentFont()
{
	LOGFONTW lf{};
	SystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);
	DBG("DefaultSansSerifTypefaceName=" << juce::String(lf.lfFaceName).quoted());
#define SELECT_TYPEFACE_BYNAME 0
#if SELECT_TYPEFACE_BYNAME
	juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName(lf.lfFaceName);
#else
	juce::Font font;
	font.setTypefaceName(lf.lfFaceName);
	juce::Typeface::Ptr tf = font.getTypefacePtr();
	juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(tf);
#endif
}
#else
void setupEnvironmentFont()
{
}
#endif

class MainComponent : public juce::Component
{
private:
	struct Renderer
	{
		juce::Component* component = nullptr;
		juce::ComponentPeer* peer = nullptr;
		juce::StringArray engineNameList;
		int engineIndex = 0;
		std::function<void()> onEngineListDidUpdate;
		std::function<void()> onEngineDidChange;
		Renderer(juce::Component* p) : component(p)
		{
		}
		void updatePeer()
		{
			juce::ComponentPeer* p = component->getPeer();
			if(peer == p) return;
			peer = p;
			if(!peer) return;
			engineNameList = peer->getAvailableRenderingEngines();
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
	juce::ComboBox fontSizeCombo;
	juce::Label label;
	juce::TextEditor textEditor;
	enum { Margin = 8, Spacing = 4, ControlHeight = 24 };
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
public:
	MainComponent() : renderer(this)
	{
//		setOpaque(true);
//		DBG("defaultalpha=" << getAlpha());
//		setAlpha(1);
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
		typefaceCombo.addItemList(juce::Font::findAllTypefaceNames(), 1);
		typefaceCombo.setText(juce::Font().getTypefacePtr()->getName(), juce::dontSendNotification);
		if(typefaceCombo.getSelectedItemIndex() < 0) typefaceCombo.setSelectedItemIndex(0, juce::dontSendNotification);
		typefaceCombo.onChange = [this]()
		{
			updateFont();
		};
		addAndMakeVisible(fontSizeCombo);
		fontSizeCombo.addItemList({ "8", "9", "10", "11", "12", "14", "16", "18", "20", "22", "24", "26", "28", "36", "48", "72" }, 1);
		fontSizeCombo.setText(juce::String(juce::Font().getHeight()), juce::dontSendNotification);
		if(fontSizeCombo.getSelectedItemIndex() < 0) fontSizeCombo.setSelectedItemIndex(0, juce::dontSendNotification);
		fontSizeCombo.onChange = [this]()
		{
			updateFont();
		};
		addAndMakeVisible(label);
		label.setJustificationType(juce::Justification::topLeft);
		addAndMakeVisible(textEditor);
		textEditor.setMultiLine(true, true);
		textEditor.setReturnKeyStartsNewLine(true);
		textEditor.setTabKeyUsedAsCharacter(true);
		textEditor.onTextChange = [this]()
		{
			label.setText(textEditor.getText(), juce::dontSendNotification);
		};
		textEditor.setText(juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getChildFile("sampletext.txt").loadFileAsString());
		updateFont();
		setSize(480, 320);
	}
	virtual ~MainComponent() override
	{
	}
	void updateFont()
	{
		juce::Font font(typefaceCombo.getText(), fontSizeCombo.getText().getFloatValue(), juce::Font::plain);
		DBG("font=" << font.getTypefaceName().quoted() << " " << font.getHeight());
		textEditor.applyFontToAllText(font);
		label.setFont(font);
	}
	virtual void parentHierarchyChanged() override
	{
		renderer.updatePeer();
	}
	virtual void resized() override
	{
		juce::Rectangle<int> rc = getLocalBounds().reduced(Margin);
		juce::Rectangle<int> rccb = rc.removeFromTop(ControlHeight);
		engineCombo.setBounds(rccb.removeFromLeft(160));
		typefaceCombo.setBounds(rccb.removeFromLeft(160));
		fontSizeCombo.setBounds(rccb.removeFromLeft(80));
		rc.removeFromTop(Spacing);
		textEditor.setBounds(rc.removeFromTop(rc.getHeight() / 2));
		label.setBounds(rc);
	}
	virtual void paint(juce::Graphics& g) override
	{
		g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	}
};

class MainWindow : public juce::DocumentWindow
{
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
public:
	MainWindow(juce::String name) : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons)
	{
		setUsingNativeTitleBar(true);
		setContentOwned(new MainComponent(), true);
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
