#include "globals.h"
#include "DspUtilities.h"
#include "CSnapshotMenu.h"
#include "effect/Effect.h"
#include "SurgeBitmaps.h"

extern CFontRef surge_minifont;

// CSnapshotMenu

CSnapshotMenu::CSnapshotMenu(const CRect& size,
                             IControlListener* listener,
                             long tag,
                             SurgeStorage* storage)
    : COptionMenu(size, listener, tag, 0)
{
   this->storage = storage;
}
CSnapshotMenu::~CSnapshotMenu()
{}

void CSnapshotMenu::draw(CDrawContext* dc)
{
   setDirty(false);
}

bool CSnapshotMenu::canSave()
{
   return false;
}

void CSnapshotMenu::populate()
{
   char txt[256];
   int main = 0, sub = 0;
   bool do_nothing = false;
   const long max_main = 16, max_sub = 256;

   TiXmlElement* sect = storage->getSnapshotSection(mtype);
   if (sect)
   {
      TiXmlElement* type = sect->FirstChild("type")->ToElement();

      while (type)
      {
         int type_id = 0;
         type->Attribute("i", &type_id);
         sub = 0;
         COptionMenu* subMenu = new COptionMenu(getViewSize(), 0, main, 0, 0, kNoDrawStyle);
         TiXmlElement* snapshot = type->FirstChild("snapshot")->ToElement();
         while (snapshot)
         {
            strcpy(txt, snapshot->Attribute("name"));

            auto actionItem = new CCommandMenuItem(CCommandMenuItem::Desc(txt,0));
            auto action = [this, snapshot, type_id](CCommandMenuItem* item) {
               this->loadSnapshot(type_id, snapshot);
            };

            actionItem->setActions(action, nullptr);
            subMenu->addEntry(actionItem);

            snapshot = snapshot->NextSibling("snapshot")->ToElement();
            sub++;
            if (sub >= max_sub)
               break;
         }

         strcpy(txt, type->Attribute("name"));
         if (sub)
         {
            addEntry(subMenu, txt);
         }
         else
         {
            auto actionItem = new CCommandMenuItem(CCommandMenuItem::Desc(txt, 0));
            auto action = [this, type_id](CCommandMenuItem* item) {
               this->loadSnapshot(type_id, nullptr);
            };

            actionItem->setActions(action, nullptr);
            addEntry(actionItem);
         }
         subMenu->forget();

         type = type->NextSibling("type")->ToElement();
         main++;
         if (main >= max_main)
            break;
      }

      /*if (canSave())
      {
         addEntry("-");
         addEntry("Store Current");
      }*/
      /*
      int32_t sel_id = contextMenu->getLastResult();
      int32_t sub_id, main_id;
      COptionMenu *b = contextMenu->getLastItemMenu(sub_id);
      if (b) main_id = b->getTag();

      if ((sel_id >= 0) && (sel_id < contextMenu->getNbEntries()))
      {
         if (sel_id < main)
         {
            int type_id = 0; xpp[sel_id&(max_main - 1)]->Attribute("i", &type_id);		//
      get "i" value from xmldata load_snapshot(type_id, 0);
         }
         else
         {
            char name[namechars];
            sprintf(name, "default");

            spawn_miniedit_text(name, namechars);
            save_snapshot(sect, name);
            storage->save_snapshots();
            do_nothing = true;
         }
      }
      else if (b && within_range(0, main_id, max_main) && within_range(0, sub_id, max_sub))
      {
         if (xp[main_id][sub_id])
         {
            int type_id = 0; xpp[main_id&(max_main - 1)]->Attribute("i", &type_id);		//
      get "i" value from xmldata

            load_snapshot(type_id, xp[main_id][sub_id]);
         }
      }
      else do_nothing = true;*/
   }
}

// COscMenu

COscMenu::COscMenu(const CRect& size,
                   IControlListener* listener,
                   long tag,
                   SurgeStorage* storage,
                   OscillatorStorage* osc)
    : CSnapshotMenu(size, listener, tag, storage)
{
   strcpy(mtype, "osc");
   this->osc = osc;
   populate();
}

void COscMenu::draw(CDrawContext* dc)
{
   CRect size = getViewSize();
   int i = osc->type.val.i;
   int y = i * size.getHeight();
   getSurgeBitmap(IDB_OSCMENU)->draw(dc, size, CPoint(0, y), 0xff);

   setDirty(false);
}

void COscMenu::loadSnapshot(int type, TiXmlElement* e)
{
   assert(within_range(0, type, num_osctypes));
   osc->queue_type = type;
   osc->queue_xmldata = e;
}

/*void COscMenu::load_snapshot(int type, TiXmlElement *e)
{
        assert(within_range(0,type,num_osctypes));
        osc->type.val.i = type;
        //osc->retrigger.val.i =
        storage->patch.update_controls(false, osc);
        if(e)
        {
                for(int i=0; i<n_osc_params; i++)
                {
                        double d; int j;
                        char lbl[256];
                        sprintf(lbl,"p%i",i);
                        if (osc->p[i].valtype == vt_float)
                        {
                                if(e->QueryDoubleAttribute(lbl,&d) == TIXML_SUCCESS) osc->p[i].val.f
= (float)d;
                        }
                        else
                        {
                                if(e->QueryIntAttribute(lbl,&j) == TIXML_SUCCESS) osc->p[i].val.i =
j;
                        }
                }
        }
}*/

// CFxMenu

const char fxslot_names[8][namechars] = {"A Insert 1", "A Insert 2", "B Insert 1", "B Insert 2",
                                         "Send FX 1",  "Send FX 2",  "Master 1",   "Master 2"};

CFxMenu::CFxMenu(const CRect& size,
                 IControlListener* listener,
                 long tag,
                 SurgeStorage* storage,
                 FxStorage* fx,
                 FxStorage* fxbuffer,
                 int slot)
    : CSnapshotMenu(size, listener, tag, storage)
{
   strcpy(mtype, "fx");
   this->fx = fx;
   this->fxbuffer = fxbuffer;
   this->slot = slot;
   populate();
}

void CFxMenu::draw(CDrawContext* dc)
{
   CRect lbox = getViewSize();
   lbox.right--;
   lbox.bottom--;
   dc->setFillColor(kBlackCColor);
   CRect f1(lbox), f2(lbox);
   f1.inset(1, 0);
   f2.inset(0, 1);
   // dc->fillRect(f1); dc->fillRect(f2);

   dc->setFontColor(kBlackCColor);
   dc->setFont(surge_minifont);
   CRect txtbox(lbox);
   txtbox.inset(2, 2);
   dc->setFillColor(kWhiteCColor);
   // dc->fillRect(txtbox);
   txtbox.inset(3, 0);
   txtbox.right -= 6;
   txtbox.top--;
   txtbox.bottom += 2;
   dc->drawString(fxslot_names[slot], txtbox, kLeftText, true);
   char fxname[namechars];
   sprintf(fxname, "%s", fxtype_abberations[fx->type.val.i]);
   dc->drawString(fxname, txtbox, kRightText, true);

   CPoint d(txtbox.right + 2, txtbox.top + 5);
   dc->drawPoint(d, kBlackCColor);
   d.x++;
   dc->drawPoint(d, kBlackCColor);
   d.y++;
   dc->drawPoint(d, kBlackCColor);
   d.y--;
   d.x++;
   dc->drawPoint(d, kBlackCColor);

   setDirty(false);
}

void CFxMenu::loadSnapshot(int type, TiXmlElement* e)
{
   if (!type)
      fxbuffer->type.val.i = type;
   if (e)
   {
      TiXmlElement* p = e->Parent()->ToElement();
      p->QueryIntAttribute("i", &type);
      assert(within_range(0, type, num_fxtypes));
      fxbuffer->type.val.i = type;
      // storage->patch.update_controls();

      Effect* t_fx = spawn_effect(type, storage, fxbuffer, 0);
      if (t_fx)
      {
         t_fx->init_ctrltypes();
         t_fx->init_default_values();
         _aligned_free(t_fx);
      }

      for (int i = 0; i < n_fx_params; i++)
      {
         double d;
         int j;
         char lbl[256], sublbl[256];
         sprintf(lbl, "p%i", i);
         if (fxbuffer->p[i].valtype == vt_float)
         {
            if (e->QueryDoubleAttribute(lbl, &d) == TIXML_SUCCESS)
               fxbuffer->p[i].set_storage_value((float)d);
         }
         else
         {
            if (e->QueryIntAttribute(lbl, &j) == TIXML_SUCCESS)
               fxbuffer->p[i].set_storage_value(j);
         }

         sprintf(sublbl, "p%i_temposync", i);
         fxbuffer->p[i].temposync =
             ((e->QueryIntAttribute(sublbl, &j) == TIXML_SUCCESS) && (j == 1));
         sprintf(sublbl, "p%i_extend_range", i);
         fxbuffer->p[i].extend_range =
             ((e->QueryIntAttribute(sublbl, &j) == TIXML_SUCCESS) && (j == 1));
      }
   }
}
void CFxMenu::saveSnapshot(TiXmlElement* e, const char* name)
{
   if (fx->type.val.i == 0)
      return;
   TiXmlElement* t = e->FirstChild("type")->ToElement();
   while (t)
   {
      int ii;
      if ((t->QueryIntAttribute("i", &ii) == TIXML_SUCCESS) && (ii == fx->type.val.i))
      {
         // if name already exists, delete old entry
         TiXmlElement* sn = t->FirstChild("snapshot")->ToElement();
         while (sn)
         {
            if (sn->Attribute("name") && !strcmp(sn->Attribute("name"), name))
            {
               t->RemoveChild(sn);
               break;
            }
            sn = sn->NextSibling("snapshot")->ToElement();
         }

         TiXmlElement neu("snapshot");

         for (int p = 0; p < n_fx_params; p++)
         {
            char lbl[256], txt[256], sublbl[256];
            sprintf(lbl, "p%i", p);
            if (fx->p[p].ctrltype != ct_none)
            {
               neu.SetAttribute(lbl, fx->p[p].get_storage_value(txt));

               if (fx->p[p].temposync)
               {
                  sprintf(sublbl, "p%i_temposync", p);
                  neu.SetAttribute(sublbl, "1");
               }
               if (fx->p[p].extend_range)
               {
                  sprintf(sublbl, "p%i_extend_range", p);
                  neu.SetAttribute(sublbl, "1");
               }
            }
         }
         neu.SetAttribute("name", name);
         t->InsertEndChild(neu);
         return;
      }
      t = t->NextSibling("type")->ToElement();
   }
}