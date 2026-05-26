# Replace `src/Qt/audiooutputwrapper.cpp` with this

```cpp
#include "audiooutputwrapper.h"
#include "qtel6.h"
#include "p6vxapp.h"

#ifndef NOSOUND
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QBuffer>
#include <QMutex>
#include <QDebug>
#include <QTimer>

#ifdef NOCALLBACK


AudioOutputWrapper::AudioOutputWrapper(const QAudioDeviceInfo &device,
									   const QAudioFormat &format,
									   CBF_SND cbFunc,
									   void *cbData,
									   int samples,
									   QObject *parent)
	: QObject(parent)
	, AudioSink(new QAudioOutput(device, format, this))
{
	AudioSink->setBufferSize(samples * bytesPerSample());
}

AudioOutputWrapper::~AudioOutputWrapper()
{
}

void AudioOutputWrapper::start()
{
	AudioBuffer = AudioSink->start();
}

void AudioOutputWrapper::suspend()
{
	AudioSink->suspend();
}

void AudioOutputWrapper::resume()
{
	AudioSink->resume();
}

void AudioOutputWrapper::stop()
{
	AudioSink->stop();
}

void AudioOutputWrapper::writeAudioStream(BYTE *stream, int samples)
{
	if (AudioBuffer){
		AudioBuffer->write(reinterpret_cast<const char*>(stream), samples * bytesPerSample());
	}
}

int AudioOutputWrapper::queuedAudioSamples()
{
	qDebug() << AudioSink->state() << AudioSink->bytesFree();
	if (AudioBuffer){
		return AudioBuffer->bytesAvailable() / bytesPerSample();
	}
	return 0;
}

int AudioOutputWrapper::bytesPerSample()
{
	return AudioSink->format().sampleSize() / 8;
}

QAudio::State AudioOutputWrapper::state() const
{
	return AudioSink->state();
}

#else

class AudioBufferWrapper : public QIODevice
{
public:
	AudioBufferWrapper(CBF_SND cbFunc,
					   void *cbData,
					   int bytesPerSample,
					   QObject* parent)
		: QIODevice(parent)
		, CbFunc(cbFunc)
		, CbData(cbData)
		, BytesPerSample(bytesPerSample)
	{}

	virtual ~AudioBufferWrapper(){
		close();
	}

	bool isSequential() const override{
		return true;
	}

	qint64 size() const override
	{
		return bytesAvailable();
	}

	qint64 bytesAvailable() const override{
		EL6* el6 = STATIC_CAST(EL6*, CbData);
		QtEL6* qtel6 = dynamic_cast<QtEL6*>(el6);
		int bytesAvailable = qtel6->GetSoundReadySize() * BytesPerSample /
							 ((double)qtel6->GetSpeedRatio() / 100.0);
		return bytesAvailable;
	}

protected:
	qint64 readData(char *data, qint64 maxlen) override
	{
		CbFunc(CbData, reinterpret_cast<BYTE*>(data), maxlen);
		return maxlen;
	}

	qint64 writeData(const char *data, qint64 len) override
	{
		return -1;
	}

private:
	CBF_SND CbFunc;
	void* CbData;
	int BytesPerSample;
};


AudioOutputWrapper::AudioOutputWrapper(
	CBF_SND cbFunc,
	void *cbData,
	int rate,
	int samples,
	QObject *parent)
	: QObject(parent)
	, ExpectedState(QAudio::StoppedState)
{
	Format.setChannelCount(1);
	Format.setSampleRate(rate);
	Format.setSampleSize(16);
	Format.setCodec("audio/pcm");
	Format.setByteOrder(QAudioFormat::LittleEndian);
	Format.setSampleType(QAudioFormat::SignedInt);

	AudioBuffer = new AudioBufferWrapper(cbFunc, cbData, 2, this);

	QTimer* recoveryTimer = new QTimer(this);
	connect(recoveryTimer, &QTimer::timeout, this, &AudioOutputWrapper::recoverPlayback);
	recoveryTimer->setInterval(1000);
	recoveryTimer->start();

	initDevice();
}

AudioOutputWrapper::~AudioOutputWrapper()
{
}

void AudioOutputWrapper::start()
{
	AudioBuffer->open(QIODevice::ReadOnly | QIODevice::Unbuffered);
	if (!AudioSink.isNull()){
		AudioSink->stop();
		AudioSink->start(AudioBuffer);
	}
	ExpectedState = QAudio::ActiveState;
}


void AudioOutputWrapper::suspend()
{
	if (!AudioSink.isNull()){
		AudioSink->suspend();
	}
	ExpectedState = QAudio::SuspendedState;
}

void AudioOutputWrapper::resume()
{
	if (!AudioSink.isNull()){
		AudioSink->resume();
	}
	ExpectedState = QAudio::ActiveState;
}

void AudioOutputWrapper::stop()
{
	if (!AudioSink.isNull()){
		AudioSink->reset();
		AudioSink->stop();
		AudioBuffer->close();
	}
	ExpectedState = QAudio::StoppedState;
}

QAudio::State AudioOutputWrapper::state() const
{
	if (!AudioSink.isNull()){
		return AudioSink->state();
	} else {
		return QAudio::StoppedState;
	}
}

void AudioOutputWrapper::initDevice()
{
	auto device = QAudioDeviceInfo::defaultOutputDevice();
	auto deviceName = device.deviceName();
	if (CurrentDevice == deviceName){
		return;
	}
	qDebug() << "AudioOutputWrapper::initDevice deviceName:" << deviceName;

	auto state = ExpectedState;
	if (!AudioSink.isNull()){
		stop();
		AudioSink->deleteLater();
	}
	AudioSink = new QAudioOutput(device, Format, this);
	qDebug()<< "AudioOutputWrapper::initDevice bufferSize:" <<AudioSink->bufferSize();
	ExpectedState = state;
	CurrentDevice = deviceName;
}

void AudioOutputWrapper::recoverPlayback()
{
	auto actualState = state();
	if (actualState != ExpectedState){
		switch (ExpectedState){
		case QAudio::ActiveState:
			start();
			break;
		default:;
		}
	}
}
#endif // NOCALLBACK
#endif
```
