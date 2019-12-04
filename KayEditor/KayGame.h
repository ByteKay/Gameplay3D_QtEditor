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

	void initializeWidgeAttribute();
	void initializeGameAttribute();
private:
	bool isMouseLeftPressed;
	QPoint mPressPos;
	int mMouseX;
	int mMouseY;
	float mYaw;
	float mPitch;
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
