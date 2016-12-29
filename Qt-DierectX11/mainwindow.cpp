#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include "d3drenderwidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    D3DRenderWidget* d3d = new D3DRenderWidget( this );
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget( d3d );
	ui->centralWidget->setLayout( pLayout );

	d3d->initialize();
}

MainWindow::~MainWindow()
{
    delete ui;
}
