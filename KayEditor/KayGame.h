#pragma once

#include "ConstantDefine.h"

class KayGame : public QWidget, public Game, public Camera::Listener
{
	Q_OBJECT
public:
	KayGame(QWidget *parent = 0);
	~KayGame();
protected:
	void initialize();

	void finalize();

	void update(float elapsedTime);

	void render(float elapsedTime);

	virtual void cameraChanged(Camera* camera);
public:
	// Game
	void Frame();
	void setTargetNode(Node* node);
	Node* getTargetNode();
	void refreshGrid(int level, float alpha) {  } // todo
	// QWidget
	bool eventFilter(QObject *obj, QEvent *event);
private:
	virtual void resizeEvent(unsigned int width, unsigned int height);
	void handleWheelEvent(QWheelEvent* evt);
	void handleMousePress(QMoveEvent* evt);
	void handleMouseRelease(QMoveEvent* evt);
	void handleMouseMove(QMoveEvent* evt);
	void handleKeyEvent(QKeyEvent* evt, bool isPress);
	void handleKeyFocusEvent(bool isPress);

	void ChangeCurrentState();
	void ChangeKeyPressedState(int key, bool isPressed);
	bool isKeyPressed(int key);

	void moveCamera(QMoveEvent* evt);
	void rotateCamera(QMoveEvent* evt);

	void initializeWidgeAttribute();
	void initializeGameAttribute();
private:
	enum KeyMouseState
	{
		NONE,
		OBJECT_CHOOSE,
		OBJECT_X_CHOOSE,
		OBJECT_Y_CHOOSE,
		OBJECT_Z_CHOOSE,
		OBJECT_XMOVE,
		OBJECT_YMOVE,
		OBJECT_ZMOVE,
		OBJECT_XROTATE,
		OBJECT_YROTATE,
		OBJECT_ZROTATE,

		CAMERA_MOVE,
		CAMERA_ROTATE,
	};
	bool isMouseLeftPressed;
	KeyMouseState mCurrentState;
	QPoint mPressPos;
	int mMouseX;
	int mMouseY;

	float mYaw;
	float mPitch;
	std::unordered_map<int, bool> mKeyRecord;
private:
	Model* mGridModel;
	Scene* mScene;
	Camera* mCamera;
	Font* mFont;
	Platform *mPlatform;
	Node* mTargetNode;

private:
	// Debug
	int mCameraChangedCount;
};
