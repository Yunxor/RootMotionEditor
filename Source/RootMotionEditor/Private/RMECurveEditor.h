#pragma once

#include "CoreMinimal.h"
#include "CurveEditorTypes.h"
#include "Tree/ICurveEditorTreeItem.h"
#include "UObject/Object.h"

struct FRMECurveEditorTreeItem : public ICurveEditorTreeItem, TSharedFromThis<FRMECurveEditorTreeItem>
{
public:
	FRMECurveEditorTreeItem(TWeakObjectPtr<UObject> InCurveOwner, FRichCurve* InCurveToEdit, FText InCurveName, FLinearColor InCurveColor);

	
	virtual TSharedPtr<SWidget> GenerateCurveEditorTreeWidget(const FName& InColumnName, TWeakPtr<FCurveEditor> InCurveEditor,
		FCurveEditorTreeItemID InTreeItemID, const TSharedRef<ITableRow>& TableRow) override;

	virtual void CreateCurveModels(TArray<TUniquePtr<FCurveModel>>& OutCurveModels) override;

private:
	TWeakObjectPtr<UObject> CurveOwner;
	FRichCurve* CurveToEdit;
	FText CurveName;
	FLinearColor CurveColor;
};

class FRMECurveEditor : public TSharedFromThis<FRMECurveEditor>
{
public:
	static const FName CurveEditorTabName;
	
	void Initialize();

	void RegisterTabSpawner(const TSharedPtr<class FTabManager>& TabManager, TSharedRef<class FRMEContext> Context);

	
protected:
	void AddNewCurve(class URMECurveContainer* Container);
	void ClearAllCurves();
	void ClearEditorAllCurves();
	void SaveCurveData();

	bool CanEditCurve() const;
	
	bool HasExternalCurve() const;
	void LoadExternalCurveData();
	void SaveToExternalCurveData();

	bool HasExternalAnim() const;
	void LoadExternalAnimData();
	void SaveToExternalAnimData();
	
private:
	TSharedRef<SWidget> CreateToolbar();
	TSharedRef<SWidget> CreateCurveEditorToolbar();
	void CreateDefaultCurves();
	void SetupCurveEditor();

	URMECurveContainer* GetCurveContainer() const;

	void CurveChanged(TWeakPtr<FRMECurveEditorTreeItem> TreeItemWeak);
	
	void AddNewCurveInternal(FVectorCurve& CurveData, UObject* CurveOwner, const FString& ChannelName);

	
private:
	TWeakPtr<FTabManager> WeakTabManager;
	
	TSharedPtr<class FCurveEditor> CurveEditor;
	TSharedPtr<class SCurveEditorPanel> CurveEditorPanel;
	TSharedPtr<FRMEContext> EditedContext;
	/** The search widget for filtering curves in the Curve Editor tree. */
	TSharedPtr<SWidget> CurveEditorSearchBox;
	/** The tree widget in the curve editor */
	TSharedPtr<class SCurveEditorTree> CurveEditorTree;

	bool bHasCurveEdited = false;
};
