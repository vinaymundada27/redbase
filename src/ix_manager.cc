#include "ix_manager.h"
#include "ix_indexhandle.h"
#include <sstream>

//
// IX_Manager
//
// Desc: Constructor - intended to be called once at begin of program
//       Handles creation, deletion, opening and closing of files.
//
IX_Manager::IX_Manager(DS_Manager & pfm):
  pfm(pfm)
{
  
}

//
// ~IX_Manager
//
// Desc: Destructor - intended to be called once at end of program
//       All files are expected to be closed when this method is called.
//
IX_Manager::~IX_Manager()
{

}

//
// CreateIndex
//
// Desc: Create a new IX table/file named fileName
// In:   fileName - name of file to create
// Ret:  IX return code
//
RC IX_Manager::CreateIndex (const char *fileName, int indexNo,
                            AttrType attrType, int attrLength,
                            int pageSize)
{
  if(indexNo < 0 ||
     attrType < INT ||
     attrType > STRING ||
     fileName == NULL)
    return IX_FCREATEFAIL;

  if(attrLength >= pageSize - (int)sizeof(RID) ||
     attrLength <= 0)
    return IX_INVALIDSIZE;

  if((attrType == INT && (unsigned int)attrLength != sizeof(int)) ||
     (attrType == FLOAT && (unsigned int)attrLength != sizeof(float))
     ||
     (attrType == STRING && 
      ((unsigned int)attrLength <= 0 || 
       (unsigned int)attrLength > MAXSTRINGLEN)))
      return IX_FCREATEFAIL;

  stringstream newname;
  newname << fileName << "." << indexNo;

  //TODO verify that RM file actually exists

  int RC = pfm.createFile((char *)newname.str().c_str());
  if (RC < 0)
  {
    // PF_PrintError(rc);
    return IX_PF;
  }
   
  DS_FileHandle pfh;
  RC = pfm.loadFile((char *)newname.str().c_str(), pfh);
  if (RC < 0)
  {
    // PF_PrintError(rc);
    return IX_PF;
  }
   
  DS_PageHandle headerPage;
  char * pData;
   
  RC = pfh.allocatePage(headerPage);
  if (RC < 0)
  {
    // PF_PrintError(rc);
    return IX_PF;
  }
  RC = headerPage.getData(pData);
  if (RC < 0)
  {
    // PF_PrintError(rc);
    return IX_PF;
  }
  IX_FileHdr hdr;
  hdr.numPages = 1; // header page
  hdr.pageSize = pageSize;
  hdr.pairSize = attrLength + sizeof(RID);
  hdr.order = -1;
  hdr.height = 0;
  hdr.rootPage = -1;
  hdr.attrType = attrType;
  hdr.attrLength = attrLength;

  memcpy(pData, &hdr, sizeof(hdr));
  //TODO - remove PF_PrintError or make it #define optional
  PageNum headerPageNum;
  headerPage.getPageNum(headerPageNum);
  assert(headerPageNum == 0);
  RC = pfh.markDirty(headerPageNum);
  if (RC < 0)
  {
    // PF_PrintError(rc);
    return IX_PF;
  }
  RC = pfh.unpinPage(headerPageNum);
  if (RC < 0)
  {
    // PF_PrintError(rc);
    return IX_PF;
  }  
  // RC = pfm.CloseFile(pfh);
  // if (RC < 0)
  // {
  //   // PF_PrintError(rc);
  //   return IX_PF;
  // }
  return (0);
}

//
// DestroyFile
//
// Desc: Delete a RM file named fileName (fileName must exist and not be open)
// In:   fileName - name of file to delete
// Ret:  RM return code
//
RC IX_Manager::DestroyIndex (const char *fileName, int indexNo)
{
  // if(indexNo < 0 ||
  //    fileName == NULL)
  //   return IX_FCREATEFAIL;

  // stringstream newname;
  // newname << fileName << "." << indexNo;

  // RC RC = pfm.DestroyFile(newname.str().c_str()); 
  // if (RC < 0)
  // {
  //   // PF_PrintError(rc);
  //   return IX_PF;
  // }
  return 0;
}

//
// OpenFile
//
// In:   fileName - name of file to open
// Out:  fileHandle - refer to the open file
//                    this function modifies local var's in fileHandle
//       to point to the file data in the file table, and to point to the
//       buffer manager object
// Ret:  PF_FILEOPEN or other RM return code
//
RC IX_Manager::OpenIndex (const char *fileName, int indexNo, IX_IndexHandle &ixh)
{
  if(indexNo < 0 ||
     fileName == NULL)
    return IX_FCREATEFAIL;

  DS_FileHandle pfh;
  stringstream newname;
  newname << fileName << "." << indexNo;

  RC rc = pfm.loadFile((char *)newname.str().c_str(), pfh);
  if (rc < 0)
  {
    // PF_PrintError(rc);
    return IX_PF;
  }
  // header page is at 0
  DS_PageHandle ph;
  char * pData;
  if ((rc = pfh.getThisPage(0, ph)) ||
      (rc = ph.getData(pData)))
    return(rc);
  IX_FileHdr hdr;
  memcpy(&hdr, pData, sizeof(hdr));
  rc = ixh.Open(&pfh, hdr.pairSize, hdr.rootPage, hdr.pageSize);
  if (rc < 0)
  {
    IX_PrintError(rc);
    return rc;
  }
  rc = pfh.unpinPage(0);
  if (rc < 0)
  {
    // PF_PrintError(rc);
    return rc;
  }

  return 0;
}

//
// CloseIndex
//
// Desc: Close file associated with fileHandle
//       The file should have been opened with OpenFile().
// In:   fileHandle - handle of file to close
// Out:  fileHandle - no longer refers to an open file
//                    this function modifies local var's in fileHandle
// Ret:  RM return code
//
RC IX_Manager::CloseIndex(IX_IndexHandle &ixh)
{
  if(!ixh.bFileOpen || ixh.pfHandle == NULL)
    return IX_FNOTOPEN;

  // cerr << "IX_Manager::CloseIndex - in method\n";
  if(ixh.HdrChanged())
  {
    // cerr << "IX_Manager::CloseIndex - header changed\n";
    // write header to disk
    DS_PageHandle ph;
    RC rc = ixh.pfHandle->getThisPage(0, ph);
    if (rc < 0)
    {
      // PF_PrintError(rc);
      return rc;
    }

    rc = ixh.SetFileHeader(ph); // write hdr into file
    if (rc < 0)
    {
      // PF_PrintError(rc);
      return rc;
    }

    rc = ixh.pfHandle->markDirty(0);
    if (rc < 0)
    {
      // PF_PrintError(rc);
      return rc;
    }

    rc = ixh.pfHandle->unpinPage(0);
    if (rc < 0)
    {
      // PF_PrintError(rc);
      return rc;
    }

    // cerr << "IX_Manager::CloseIndex - header changed\n";
    // cerr << ixh.root->GetNumKeys() << "-- root keys\n";
    rc = ixh.ForcePages();
    if (rc < 0)
    {
      IX_PrintError(rc);
      return rc;
    }
  }
      
  // RC rc2 = pfm.CloseFile(*ixh.pfHandle);
  // if (rc2 < 0) {
  //   PF_PrintError(rc2);
  //   return rc2;
  // }
  // ixh.~IX_IndexHandle();
  // ixh.pfHandle = NULL;
  // ixh.bFileOpen = false;
  return 0;
}
