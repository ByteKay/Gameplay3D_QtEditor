#include "KayGame.h"
#include "KayUtils.h"

KayGame::KayGame(QWidget *parent) : QWidget(parent), Game(), isMouseLeftPressed(false), mMouseX(0), mMouseY(0), mPitch(0), mYaw(0)
{
	initializeWidgeAttribute();
	initializeGameAttribute();
	mCameraChangedCount = 0;
	mCurrentState = KeyMouseState::NONE;
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

	mYaw = 0;
	mPitch = -MATH_PIOVER4;
	Quaternion q;
	Quaternion::createFromEuler(mYaw, mPitch, 0, &q);
	cameraNode->setRotation(q);

	mGridModel = KayUtils::CreateGridModel(1000);
	Node* gridNode = mScene->addNode();
	gridNode->setDrawable(mGridModel);

	mCamera->addListener(this);
}

void KayGame::update(float elapsedTime)
{

}

void KayGame::render(float elapsedTime)
{
	char buffer[256];
	sprintf(buffer, "FPS: %d, CameraChenged: %d", getFrameRate(), mCameraChangedCount);
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

void KayGame::setTargetNode(Node* node)
{
	mTargetNode = node;
}

Node* KayGame::getTargetNode()
{
	if (mTargetNode)
	{
		return mTargetNode;
	}
	return NULL;
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

// 可以记录一个状态
void KayGame::handleKeyEvent(QKeyEvent* evt, bool isPress)
{
	ChangeKeyPressedState(evt->key(), isPress);
	switch (evt->key())
	{
	case Qt::Key_F:
		handleKeyFocusEvent(isPress);
		break;
	case Qt::Key_Alt:

		break;
	default:
		break;
	}
}

void KayGame::ChangeKeyPressedState(int key, bool isPressed)
{
	mKeyRecord[key] = isPressed;
}

bool KayGame::isKeyPressed(int key)
{
	if (mKeyRecord.find(key) != mKeyRecord.end())
	{
		return mKeyRecord[key];
	}
	return false;
}

void KayGame::ChangeCurrentState()
{
	if (mCurrentState != KeyMouseState::NONE)
	{
		return;
	}
	if (isKeyPressed(Qt::Key_Alt))
	{
		if (isKeyPressed(Qt::Key_Control))
		{
			mCurrentState = KeyMouseState::CAMERA_MOVE;
		}
		else
		{
			mCurrentState = KeyMouseState::CAMERA_ROTATE;
		}
	}
}

void KayGame::handleKeyFocusEvent(bool isPress)
{
	Node* node = mGridModel->getNode();
	if (!isPress && node)
	{
		static int times = 5;
		float radius = node->getBoundingSphere().radius;
		float distance = times * (radius < 1 ? 1 : radius);
		Vector3 modelPos = node->getTranslationWorld();
		Vector3 cameraPos = modelPos - distance * mCamera->getNode()->getForwardVectorWorld();
		mCamera->getNode()->setTranslation(cameraPos);
	}
}

void KayGame::handleWheelEvent(QWheelEvent* evt)
{
	mCamera->getNode()->translateForward(evt->delta() * 0.01f);
}

void KayGame::handleMousePress(QMoveEvent* evt)
{
	isMouseLeftPressed = true;
	mPressPos = QCursor::pos();
}

void KayGame::handleMouseRelease(QMoveEvent* evt)
{
	isMouseLeftPressed = false;
	mCurrentState = KeyMouseState::NONE;;
}

void KayGame::moveCamera(QMoveEvent* evt)
{
	const int oldx = mPressPos.x();
	const int oldy = mPressPos.y();
	mPressPos = QCursor::pos();
	float x = (mPressPos.x() - oldx) * 0.12f;
	float y = (mPressPos.y() - oldy) * 0.25f;
	mCamera->getNode()->translateUp(y);
	mCamera->getNode()->translateLeft(x);
}

void KayGame::rotateCamera(QMoveEvent* evt)
{
	const int oldx = mPressPos.x();
	const int oldy = mPressPos.y();
	mPressPos = QCursor::pos();
	float yAngle = -(mPressPos.x() - oldx) * 0.0012f;
	float xAngle = -(mPressPos.y() - oldy) * 0.0025f;

	Node* node = mCamera->getNode();
	Ray r(node->getTranslationWorld(), node->getForwardVectorWorld());
	Plane p(Vector3::unitY(), 0);
	float d = r.intersects(p);
	if (d > 0)
	{
		Vector3 modelPos = r.getOrigin() + r.getDirection() * d;
		mYaw += yAngle;
		mPitch += xAngle;
		mPitch = mPitch > MATH_PIOVER2 ? MATH_PIOVER2 : (mPitch < -MATH_PIOVER2 ? -MATH_PIOVER2 : mPitch);
		Quaternion q;
		Quaternion::createFromEuler(mYaw, mPitch, 0, &q);
		node->setRotation(q);
		node->setTranslation(modelPos);
		Vector3 direction = node->getForwardVectorWorld();
		direction.normalize();
		direction = direction * (-d);
		node->translate(direction);
	}
	else
	{

	}

}

void KayGame::handleMouseMove(QMoveEvent* evt)
{
	if (isMouseLeftPressed)
	{
		ChangeCurrentState();
	}
	switch (mCurrentState)
	{
	case KayGame::NONE:
		break;
	case KayGame::OBJECT_CHOOSE:
		break;
	case KayGame::OBJECT_X_CHOOSE:
		break;
	case KayGame::OBJECT_Y_CHOOSE:
		break;
	case KayGame::OBJECT_Z_CHOOSE:
		break;
	case KayGame::OBJECT_XMOVE:
		break;
	case KayGame::OBJECT_YMOVE:
		break;
	case KayGame::OBJECT_ZMOVE:
		break;
	case KayGame::OBJECT_XROTATE:
		break;
	case KayGame::OBJECT_YROTATE:
		break;
	case KayGame::OBJECT_ZROTATE:
		break;
	case KayGame::CAMERA_MOVE:
		moveCamera(evt);
		break;
	case KayGame::CAMERA_ROTATE:
		rotateCamera(evt);
		break;
	default:
		break;
	}
}

bool KayGame::eventFilter(QObject *obj, QEvent *event)
{
	switch (event->type())
	{
	case QEvent::KeyPress:
		handleKeyEvent((QKeyEvent*)event, true);
		break;
	case QEvent::KeyRelease:
		handleKeyEvent((QKeyEvent*)event, false);
		break;
	case QEvent::Wheel:
		handleWheelEvent((QWheelEvent*)event);
		break;
	case QEvent::MouseButtonPress:
		handleMousePress((QMoveEvent*)event);
		break;
	case QEvent::MouseMove:
		handleMouseMove((QMoveEvent*)event);
		break;
	case QEvent::MouseButtonRelease:
		handleMouseRelease((QMoveEvent*)event);
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

void KayGame::cameraChanged(Camera* camera)
{
	++mCameraChangedCount;
	// 主要针对 相机移动位置变化 处理
	// 限制在一定范围内
	// 网格需要动态改变
	// todo
}

