#ifndef SUBTREE_DIALOG_H
#define SUBTREE_DIALOG_H

#include <QtGui>
#include "v3d_basicdatatype.h"
#include "basic_surf_objs.h"
#include "v3d_interface.h"
#include "math.h"
#include <vector>

using namespace std;

class subtree_dialog:public QDialog
{
    Q_OBJECT
public:
   explicit subtree_dialog(V3DPluginCallback2 *cb);
   //void create();
   bool get_swc_marker();
   void subtree_extract();
   void assign_marker_type();
   void soma_clicked();
public slots:
   void marker_change();

private:
   int calc_nearest_node_around_marker();
   void type_def(int type, int dendrite_id, int marker_id);
private:
   V3DPluginCallback2 *callback;
   LandmarkList LList_in;
   NeuronTree nt;
   V3DLONG sz_img[4];
   V3dR_MainWindow *swc_win;
   vector<vector<int> > subtree;
   QComboBox *markers;
};

#endif // SUBTREE_DIALOG_H

