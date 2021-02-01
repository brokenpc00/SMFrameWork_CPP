//
//  SMVideoPlayer.cpp
//  IPCT
//
//  Created by SteveKim on 2019/12/31.
//

#include "SMVideoPlayer.h"
#include "../Base/SMImageView.h"
#include "../Base/SMButton.h"
#include "ViewUtil.h"
#include "VideoSprite.h"
#include <cocos2d.h>
#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
#import <OpenAL/alc.h>
#endif

SMVideoPlayer::SMVideoPlayer() : _contentView(nullptr)
, _videoImageView(nullptr)
, _isPlayable(false)
, _audioButton(nullptr)
, _recordButton(nullptr)
, _rotateButton(nullptr)
, _playButton(nullptr)
, _screenshotButton(nullptr)
, _playerStatus(SMPlayerStatus::STOPPED)
, _playerType(SMPlayerType::NORMAL)
, _playerRotate(SMPlayerRotate::VERTICAL)
, _visibleControl(false)
, _isAudioPlay(true)
{
    
}

SMVideoPlayer::~SMVideoPlayer()
{
    
}

bool SMVideoPlayer::initWithUrl(const std::string& videoSource)
{
    if (!SMView::init()) {
        return false;
    }
    
    auto s = getContentSize();
    auto screen = cocos2d::Director::getInstance()->getWinSize();
    
    bool isFullScreenSize = s.width==screen.width&&s.height==screen.height;
    
    _contentView = SMView::create(0, 0, 0, s.width, s.height);
    addChild(_contentView);
    _contentView->setOnClickListener(this);
    
    _videoImageView = SMImageView::create();
    _videoImageView->setPosition(0, 0);
    _videoImageView->setAnchorPoint(cocos2d::Vec2::ZERO);
    _videoImageView->setContentSize(s);
    
    VideoSprite::createWithImageView(videoSource, _videoImageView);
    
    _videoImageView->setScaleType(SMImageView::ScaleType::CENTER_CROP);
    _contentView->addChild(_videoImageView);

    _visibleControl = false;
    if (isFullScreenSize) {
        // audio button (except STREAM type)
        if (_playerType!=SMPlayerType::STREAM) {
            // rotate -> not yet...
            // screenshot button -> not yet...
            // audio
            _audioButton = SMButton::create(0, SMButton::Style::DEFAULT, s.width-160, s.height-260, 120, 120);
            _audioButton->setIcon(SMButton::State::NORMAL, "images/audio_icon.png");
            _audioButton->setIconColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xeeeff1, 1));
            _audioButton->setIconColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1));
            _audioButton->setIconScale(0.5f);
            _contentView->addChild(_audioButton);
            _audioButton->setOnClickListener(this);
            _visibleControl = true;
        }
        
        // play & pause button (NORMAL type)
        if (_playerType==SMPlayerType::NORMAL) {
            // play button
            _visibleControl = true;
        }
        
        // record butron (RECORD, AUDIO_RECORD_ONLY type)
        if (_playerType==SMPlayerType::RECORD || _playerType==SMPlayerType::RECORD_AUDIO_ONLY) {
            // record button
            _visibleControl = true;
        }
    }
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    
//    _videoPlayer = cocos2d::experimental::ui::VideoPlayer::create();
//    _videoPlayer->setContentSize(cocos2d::Size::ZERO);
//    _videoPlayer->addEventListener(CC_CALLBACK_2(SMVideoPlayer::videoEventCallback, this));
//    _contentView->addChild(_videoPlayer, 1);
//    _videoPlayer->setSwallowTouches(false);
//    _videoPlayer->setURL(videoSource);
//    _videoPlayer->setStyle(cocos2d::experimental::ui::VideoPlayer::StyleType::NONE);
//    _videoPlayer->play();
    
#endif

    
    return true;
}
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
void SMVideoPlayer::videoEventCallback(cocos2d::Ref* sender, cocos2d::experimental::ui::VideoPlayer::EventType eventType)
{
    
}
#endif

void SMVideoPlayer::onEnter()
{
    SMView::onEnter();
    
    // to do...
}

void SMVideoPlayer::onExit()
{
    // to do...
    stop();
    
    SMView::onExit();
}

void SMVideoPlayer::setContentSize(const cocos2d::Size &contentSize)
{
    SMView::setContentSize(contentSize);
    
    if (_videoImageView) {
        _videoImageView->setContentSize(contentSize);
    }
}

void SMVideoPlayer::pause()
{
    cocos2d::Node::pause();
    
    // to do pause;
    pausePlay();
}

void SMVideoPlayer::resume()
{
    // to do resume
    
    cocos2d::Node::resume();
    resumePlay();
}

void SMVideoPlayer::onClick(SMView * view)
{
    if (view==_audioButton) {
        _isAudioPlay = !_isAudioPlay;
        if (_isAudioPlay) {
            _audioButton->setIconColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xeeeff1, 1));
            _audioButton->setIconColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1));
            startAudioFn();
        } else {
            _audioButton->setIconColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1));
            _audioButton->setIconColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xeeeff1, 1));
            stopAudioFn();
        }
    } else if (view==_playButton) {
        
    } else if (view==_recordButton) {
        
    } else if (view==_rotateButton) {
        
    } else if (view==_screenshotButton) {
        
    } else {
        if (_playerType==SMPlayerType::NORMAL) {
            if (_playButton) {
                cocos2d::ActionInterval * fade=nullptr;
                if (_visibleControl) {
                    fade = cocos2d::FadeOut::create(0.15f);
                } else {
                    fade = cocos2d::FadeIn::create(0.15f);
                    _playButton->setVisible(true);
                }
                auto seq = cocos2d::Sequence::create(fade, cocos2d::CallFunc::create([this]{
                    _playButton->setVisible(_visibleControl);
                }), NULL);
                _playButton->runAction(seq);
                _isAudioPlay = !_visibleControl;
                if (_isAudioPlay) {
                    stopAudioFn();
                } else {
                    
                }
            }
            if (_audioButton) {
                cocos2d::ActionInterval * fade=nullptr;
                if (_visibleControl) {
                    fade = cocos2d::FadeOut::create(0.15f);
                } else {
                    fade = cocos2d::FadeIn::create(0.15f);
                    _audioButton->setVisible(true);
                }
                auto seq = cocos2d::Sequence::create(fade, cocos2d::CallFunc::create([this]{
                    _audioButton->setVisible(_visibleControl);
                }), NULL);
                _audioButton->runAction(seq);
            }
            if (_recordButton) {
                cocos2d::ActionInterval * fade=nullptr;
                if (_visibleControl) {
                    fade = cocos2d::FadeOut::create(0.15f);
                } else {
                    fade = cocos2d::FadeIn::create(0.15f);
                    _recordButton->setVisible(true);
                }
                auto seq = cocos2d::Sequence::create(fade, cocos2d::CallFunc::create([this]{
                    _recordButton->setVisible(_visibleControl);
                }), NULL);
                _recordButton->runAction(seq);
            }
            if (_rotateButton) {
                cocos2d::ActionInterval * fade=nullptr;
                if (_visibleControl) {
                    fade = cocos2d::FadeOut::create(0.15f);
                } else {
                    fade = cocos2d::FadeIn::create(0.15f);
                    _rotateButton->setVisible(true);
                }
                auto seq = cocos2d::Sequence::create(fade, cocos2d::CallFunc::create([this]{
                    _rotateButton->setVisible(_visibleControl);
                }), NULL);
                _rotateButton->runAction(seq);
            }
            if (_screenshotButton) {
                cocos2d::ActionInterval * fade=nullptr;
                if (_visibleControl) {
                    fade = cocos2d::FadeOut::create(0.15f);
                } else {
                    fade = cocos2d::FadeIn::create(0.15f);
                    _screenshotButton->setVisible(true);
                }
                auto seq = cocos2d::Sequence::create(fade, cocos2d::CallFunc::create([this]{
                    _screenshotButton->setVisible(_visibleControl);
                }), NULL);
                _screenshotButton->runAction(seq);
            }
            
            _visibleControl=!_visibleControl;
        }
    }
}

// not using

void SMVideoPlayer::changeHorizontalScreen(bool horizontal)
{
    
}

void SMVideoPlayer::beginTransform()
{
    
}

void SMVideoPlayer::endTransform()
{
    
}

void SMVideoPlayer::hideBtnView()
{
    
}

// video & audio
void SMVideoPlayer::play()
{
    if (_isPlayable) {
        startVideoFn();
        startAudioFn();
    }
}

void SMVideoPlayer::pausePlay()
{
    if (_isPlayable) {
        stopVideoFn();
        stopAudioFn();
    }
}

void SMVideoPlayer::resumePlay()
{
    if (_isPlayable) {
        startVideoFn();
        startAudioFn();
    }
}

void SMVideoPlayer::stop()
{
    if (_isPlayable) {
        stopVideoFn();
        stopAudioFn();
    }
}

void SMVideoPlayer::startAudioFn()
{
    _isAudioPlay = true;
    
}

void SMVideoPlayer::stopAudioFn()
{
    _isAudioPlay = false;

}

void SMVideoPlayer::startVideoFn()
{
    auto vs = dynamic_cast<VideoSprite*>(_videoImageView->getSprite());
    if (vs) {
        vs->play();
    }
}

void SMVideoPlayer::stopVideoFn()
{
    // vidoe sprite stop
    auto vs = dynamic_cast<VideoSprite*>(_videoImageView->getSprite());
    if (vs) {
        vs->stop();
    }
}

void SMVideoPlayer::flush()
{
    // What am I going to do??
}
