#pragma once

#include "JuceHeader.h"

#include "../../shared/components/ear_combo_box.hpp"
#include "../../shared/components/onboarding.hpp"
#include "../../shared/components/overlay.hpp"
#include "../../shared/helper/properties_file.hpp"
#include "../../shared/components/ear_header.hpp"
#include "detail/constants.hpp"
#include "value_box_channel_gain.hpp"
#include "value_box_main.hpp"
#include "value_box_metadata.hpp"
#include "value_box_speaker_layer.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class DirectSpeakersComponent : public Component,
                                private ear::plugin::ui::Onboarding::Listener {
 public:
  DirectSpeakersComponent(DirectSpeakersAudioProcessor* p)
      : p_(p),
        header(std::make_unique<EarHeader>()),
        onBoardingButton(std::make_unique<EarButton>()),
        onBoardingOverlay(std::make_unique<Overlay>()),
        onBoardingContent(std::make_unique<Onboarding>()),
        mainValueBox(std::make_unique<ValueBoxMain>()),
        metadataValueBox(std::make_unique<ValueBoxMetadata>()),
        channelGainValueBox(
            std::make_shared<ValueBoxChannelGain>(p->getLevelMeter())),
        upperLayerValueBox(
            std::make_shared<ValueBoxSpeakerLayer>("Upper Layer")),
        middleLayerValueBox(
            std::make_shared<ValueBoxSpeakerLayer>("Middle Layer")),
        bottomLayerValueBox(
            std::make_shared<ValueBoxSpeakerLayer>("Bottom Layer")),
        statusBarLabel(std::make_shared<Label>()),
        propertiesFileLock(
            std::make_unique<InterProcessLock>("EPS_preferences")),
        propertiesFile(getPropertiesFile(propertiesFileLock.get())) {
    header->setText(" Direct Speakers");

    onBoardingButton->setButtonText("?");
    onBoardingButton->setShape(EarButton::Shape::Circular);
    onBoardingButton->setFont(
        font::RobotoSingleton::instance().getRegular(20.f));
    onBoardingButton->onClick = [&]() { onBoardingOverlay->setVisible(true); };

    onBoardingOverlay->setContent(onBoardingContent.get());
    onBoardingOverlay->setWindowSize(706, 596);
    onBoardingOverlay->setHeaderText(
        String::fromUTF8("Welcome – do you need help?"));
    onBoardingOverlay->onClose = [&]() {
      onBoardingOverlay->setVisible(false);
      propertiesFile->setValue("showOnboarding", false);
    };
    if (!propertiesFile->containsKey("showOnboarding")) {
      onBoardingOverlay->setVisible(true);
    }
    onBoardingContent->addListener(this);

    addAndMakeVisible(header.get());
    addAndMakeVisible(onBoardingButton.get());
    addChildComponent(onBoardingOverlay.get());

    //addAndMakeVisible(metadataValueBox.get());
    addAndMakeVisible(mainValueBox.get());
    addAndMakeVisible(channelGainValueBox.get());

    metadataValueBox->setEnabled(false);
    metadataValueBox->setAlpha(0.38f);

    upperLayerValueBox->setLayer(ear::plugin::Layer::upper);
    middleLayerValueBox->setLayer(ear::plugin::Layer::middle);
    bottomLayerValueBox->setLayer(ear::plugin::Layer::bottom);

    addAndMakeVisible(upperLayerValueBox.get());
    addAndMakeVisible(middleLayerValueBox.get());
    addAndMakeVisible(bottomLayerValueBox.get());

    statusBarLabel->setFont(EarFonts::Measures);
    addAndMakeVisible(statusBarLabel.get());
  }

  ~DirectSpeakersComponent() {}

  void paint(Graphics& g) override {
    g.fillAll(EarColours::Background);
    Shadows::elevation04dp.drawForRectangle(g, mainValueBox->getBounds());
    //Shadows::elevation04dp.drawForRectangle(g, metadataValueBox->getBounds());
    Shadows::elevation01dp.drawForRectangle(g,
                                            channelGainValueBox->getBounds());
  }

  void resized() override {
    auto area = getLocalBounds();
    onBoardingOverlay->setBounds(area);
    area.reduce(5, 5);
    auto headingArea = area.removeFromTop(55);
    statusBarLabel->setBounds(area.removeFromBottom(30));
    onBoardingButton->setBounds(
        headingArea.removeFromRight(39).removeFromBottom(39));
    header->setBounds(headingArea);

    auto leftColumn = area.withTrimmedRight(area.getWidth() / 2);
    auto rightColumn = area.withTrimmedLeft(area.getWidth() / 2);

    // left column
    mainValueBox->setBounds(leftColumn.removeFromTop(197).reduced(5, 5));
    channelGainValueBox->setBounds(leftColumn.reduced(5, 5));

    // right column
    /* TODO - old positions before removing metadata
    metadataValueBox->setBounds(rightColumn.removeFromTop(161).reduced(5, 5));
    upperLayerValueBox->setBounds(rightColumn.removeFromTop(222).reduced(5, 5));
    middleLayerValueBox->setBounds(
        rightColumn.removeFromTop(222).reduced(5, 5));
    bottomLayerValueBox->setBounds(
        rightColumn.removeFromTop(222).reduced(5, 5));
        */
    upperLayerValueBox->setBounds(rightColumn.removeFromTop(278).reduced(5, 5));
    middleLayerValueBox->setBounds(
        rightColumn.removeFromTop(279).reduced(5, 5));
    bottomLayerValueBox->setBounds(
        rightColumn.removeFromTop(278).reduced(5, 5));
  }

  std::unique_ptr<EarHeader> header;
  std::unique_ptr<ear::plugin::ui::EarButton> onBoardingButton;
  std::unique_ptr<ear::plugin::ui::Overlay> onBoardingOverlay;
  std::unique_ptr<ear::plugin::ui::Onboarding> onBoardingContent;
  std::unique_ptr<ear::plugin::ui::ValueBoxMain> mainValueBox;
  std::unique_ptr<ear::plugin::ui::ValueBoxMetadata> metadataValueBox;
  std::shared_ptr<ear::plugin::ui::ValueBoxChannelGain> channelGainValueBox;
  std::shared_ptr<ear::plugin::ui::ValueBoxSpeakerLayer> upperLayerValueBox;
  std::shared_ptr<ear::plugin::ui::ValueBoxSpeakerLayer> middleLayerValueBox;
  std::shared_ptr<ear::plugin::ui::ValueBoxSpeakerLayer> bottomLayerValueBox;
  std::shared_ptr<Label> statusBarLabel;

  std::unique_ptr<InterProcessLock> propertiesFileLock;
  std::unique_ptr<PropertiesFile> propertiesFile;

  // --- Onboarding::Listener
  void endButtonClicked(Onboarding* onboarding) override {
    onBoardingOverlay->setVisible(false);
    propertiesFile->setValue("showOnboarding", false);
  }
  void moreButtonClicked(Onboarding* onboarding) override {
    onBoardingOverlay->setVisible(false);
    URL(detail::MORE_INFO_URL).launchInDefaultBrowser();
  }

 private:
  DirectSpeakersAudioProcessor* p_;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectSpeakersComponent)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
