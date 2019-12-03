#pragma once

#include "ConstantDefine.h"

class KayGame : public QWidget, public Game
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
public:
	void Frame();
	bool eventFilter(QObject *obj, QEvent *event);
private:
	virtual void resizeEvent(unsigned int width, unsigned int height);
	void handleWheelEvent(QWheelEvent* evt);
	void handleMousePress(QMoveEvent* evt);
	void handleMouseRelease(QMoveEvent* evt);
	void handleMouseMove(QMoveEvent* evt);

	void initializeWidgeAttribute();
	void initializeGameAttribute();
private:
	bool isMouseLeftPressed;
	QPoint mPressPos;
	int mMouseX;
	int mMouseY;
	float mTurn;
	float mPitch;
private:
	Model* mGridModel;
	Scene* mScene;
	Camera* mCamera;
	Font* mFont;
	Platform *mPlatform;
};
