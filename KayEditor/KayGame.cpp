#include "KayGame.h"
#include "KayUtils.h"

KayGame::KayGame(QWidget *parent) : QWidget(parent), Game(), isMouseLeftPressed(false), mMouseX(0), mMouseY(0)
{
	initializeWidgeAttribute();
	initializeGameAttribute();
}

void KayGame::initializeWidgeAttribute()
{
	setMouseTracking(true);
	setAcceptDrops(true);
	setContextMenuPolicy(Qt::PreventContextMenu);
	setFocusPolicy(Qt::ClickFocus);
	setAttribute(Qt::WA_NoBackground);
	setAttribute(Qt::WA_PaintOnScreen);
	startTimer(1, Qt::PreciseTimer);
	installEventFilter(this);
}

void KayGame::initializeGameAttribute()
{
	HWND wId = (HWND)winId();
	Platform::SetInfo(wId, L"GameplayEditor");
	mPlatform = Platform::create(Game::getInstance());
}

KayGame::~KayGame()
{
	mPlatform->shutdownInternal();
	if (mPlatform)
	{
		delete mPlatform;
		mPlatform = 0;
	}
}

void KayGame::initialize()
{
	mFont = Font::create("res/ui/arial.gpb");
	mScene = Scene::create("Test");
	mCamera = Camera::createPerspective(60.f, 1.0f, 0.1f, 1000.0f);
	mScene->setActiveCamera(mCamera);
	Node* cameraNode = mScene->addNode();
	cameraNode->setCamera(mCamera);
	cameraNode->setTranslation(0.0f, 100.0f, 0.0f);

	Quaternion q;
	Quaternion::createFromAxisAngle(Vector3::unitX(), 30, &q);
	cameraNode->setRotation(q);


	mGridModel = KayUtils::CreateGridModel(1000, 1000);
	Node* gridNode = mScene->addNode();
	gridNode->setDrawable(mGridModel);
}

void KayGame::update(float elapsedTime)
{

}

void KayGame::render(float elapsedTime)
{
	char buffer[256];
	sprintf(buffer, "FPS: %d, Tri: %d", getFrameRate(), 0);
	mFont->start();
	mFont->drawText(buffer, 0, 0, Vector4(1.0f, 0.0f, 0.0f, 1.0f), 24);
	mFont->finish();

	mGridModel->draw();
}

void KayGame::finalize()
{
	SAFE_RELEASE(mGridModel);
	SAFE_RELEASE(mCamera);
	SAFE_RELEASE(mScene);
	SAFE_RELEASE(mFont);
}

void KayGame::Frame()
{
	mPlatform->paintGL();
}

void KayGame::resizeEvent(unsigned int width, unsigned int height)
{
	Rectangle viewPort(width, height);
	setViewport(viewPort);
	if (mCamera)
	{
		float aspect = width * 1.0f / height;
		mCamera->setAspectRatio(aspect);
	}
}

void KayGame::handleWheelEvent(QWheelEvent* evt)
{
	Camera *camera = mCamera;
	Node *node = camera->getNode();
	const Vector3 cameraPos = node->getBoundingSphere().center;
	Model *model = mGridModel;
	Node * modelNode = model->getNode();
	Vector3 modelPos = modelNode->getBoundingSphere().center;
	float radius = modelNode->getBoundingSphere().radius;
	Vector3 dist = modelPos - cameraPos;
	float leans = dist.length();
	node->translate(evt->delta() * 0.0005f * dist);
}

void KayGame::handleMousePress(QMoveEvent* evt)
{
	isMouseLeftPressed = true;
	mPressPos = QCursor::pos();
}

void KayGame::handleMouseRelease(QMoveEvent* evt)
{
	isMouseLeftPressed = false;
}

void KayGame::handleMouseMove(QMoveEvent* evt)
{
	try
	{
		if (isMouseLeftPressed)
		{
			Vector3 modelPos = mGridModel->getNode()->getTranslationWorld();

			Node* node = mCamera->getNode();
			const Vector3 cameraPos = node->getTranslationWorld();
			Vector3 dist = cameraPos - modelPos;
			float leans = dist.length();
			if (leans < 0.001f)
			{
				return;
			}

			const int oldx = mPressPos.x();
			const int oldy = mPressPos.y();
			mPressPos = QCursor::pos();

			float yAngle = -(mPressPos.x() - oldx) * 1.2f * 0.005f;
			float xAngle = -(mPressPos.y() - oldy) * 2.5f * 0.005f;

			mTurn += yAngle;
			mPitch += xAngle;

			node->setTranslation(modelPos);
			node->setRotation(Quaternion::identity());
			node->rotateY(mTurn);
			node->rotateX(mPitch);

			Vector3 direction = node->getForwardVectorWorld();
			direction.normalize();
			direction = direction * (-leans);
			node->translate(direction);
		}
	}
	catch (...)
	{

	}
}


bool KayGame::eventFilter(QObject *obj, QEvent *event)
{
	switch (event->type())
	{
	case QEvent::Wheel:
		handleWheelEvent((QWheelEvent*)event);
		break;
	case QEvent::MouseButtonPress:
		handleMousePress((QMoveEvent*)event);
		break;
	case QEvent::MouseButtonRelease:
		handleMouseRelease((QMoveEvent*)event);
		break;
	case QEvent::MouseMove:
		handleMouseMove((QMoveEvent*)event);
		break;
	case QEvent::Timer:
		Frame();
		break;
	case QEvent::Resize:
		QResizeEvent* evt = (QResizeEvent*)event;
		const QSize& newSize = evt->size();
		Platform::resizeEventInternal(newSize.width(), newSize.height());
		break;
	}
	return true;
}